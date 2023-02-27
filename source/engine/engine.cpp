#include <limits>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "device.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "pipeline.hpp"
#include "utils.hpp"
#include "shaders.hpp"

namespace engine {

    Engine::Engine (GLFWwindow* window) {

        LOG_INFO("Creating Engine instance...");

        if constexpr (debug) {

            auto version = vk::enumerateInstanceVersion();

            auto major = VK_API_VERSION_MAJOR(version);
            auto minor = VK_API_VERSION_MINOR(version);
            auto patch = VK_API_VERSION_PATCH(version);

            LOG_INFO("System support up to Vulkan {}.{}.{}", major, minor, patch);

        }

        device = std::make_shared<Device>(window);

        if constexpr (debug) {
            dldi = vk::DispatchLoaderDynamic(device->get_instance(), vkGetInstanceProcAddr);
            debug_messenger = make_debug_messenger(device->get_instance(), dldi);
        }

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        graphics_queue = device->get_handle().getQueue(indices.graphics_family.value(), 0);
        present_queue = device->get_handle().getQueue(indices.present_family.value(), 0);

        prepare();

    }

    Engine::~Engine ( ) {

        device->get_handle().waitIdle();

        LOG_INFO("Destroying Engine...");

        if (debug) device->get_instance().destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        LOG_INFO("Destroying Command Pool");
        device->get_handle().destroyCommandPool(command_pool);
        LOG_INFO("Destroying Descriptor Pool");
        device->get_handle().destroyDescriptorPool(descriptor_pool);

        LOG_INFO("Destroying Pipeline");
        device->get_handle().destroyRenderPass(renderpass);
        device->get_handle().destroyPipelineLayout(pipeline_layout);
        device->get_handle().destroyDescriptorSetLayout(descriptor_set_layout);
        device->get_handle().destroyPipeline(pipeline);

    }

    void Engine::prepare ( ) {

        make_command_pool();

        renderpass = create_renderpass(device);

        swapchain = std::make_unique<SwapChain>(device, renderpass);
        max_frames_in_flight = swapchain->get_frames().size();

        make_descriptor_pool();
        descriptor_set_layout = create_descriptor_set_layout(device);

        swapchain->make_commandbuffers(command_pool);
        swapchain->make_descriptor_sets(descriptor_pool, descriptor_set_layout);

        pipeline_layout = create_pipeline_layout(device, descriptor_set_layout);
        pipeline = create_pipeline(device, pipeline_layout, renderpass);

        if (is_imgui_enabled) imgui = std::make_unique<ImGUI>(device, max_frames_in_flight, renderpass);

        frame_number = 0;

        asset = std::make_unique<Mesh>(device);

    }

    void Engine::remake_swapchain ( ) {

        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(const_cast<GLFWwindow*>(device->get_window()), &width, &height);
            glfwWaitEvents();
        }

        if (width == device->get_extent().width 
            && height == device->get_extent().height)
            return;

        device->get_handle().waitIdle();

        swapchain->create_handle();

        frame_number = 0;
        is_framebuffer_resized = false;

    }

    void Engine::make_command_pool ( ) {

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = indices.graphics_family.value()
        };

        try {
            command_pool = device->get_handle().createCommandPool(create_info);
            LOG_INFO("Successfully created Command Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Command Pool");
        }

    }

    void Engine::make_descriptor_pool ( ) {

        auto pool_sizes = std::array {
            vk::DescriptorPoolSize {
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = max_frames_in_flight
            },
            vk::DescriptorPoolSize {
                .type = vk::DescriptorType::eStorageBuffer,
                .descriptorCount = max_frames_in_flight
            }
        };

        auto create_info = vk::DescriptorPoolCreateInfo {
            .flags = vk::DescriptorPoolCreateFlags(),
            .maxSets = max_frames_in_flight,
            .poolSizeCount = to_u32(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data()
        };

        try {
            descriptor_pool = device->get_handle().createDescriptorPool(create_info);
            LOG_INFO("Successfully created Descriptor Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Descriptor Pool");
        }

    }

    void Engine::prepare_frame (uint32_t index, Scene& scene) {

        auto& frame = swapchain->get_frames().at(index);

        glm::vec3 eye = { 2.0f, 2.0f, -1.0f };
        glm::vec3 center = { 0.0f, 0.0f, 0.0f };
        glm::vec3 up = { 0.0f, 0.0f, -1.0f };

        auto aspect = static_cast<float>(swapchain->get_extent().width) / static_cast<float>(swapchain->get_extent().height);

        auto ubo = UniformBufferObject {
            .view = glm::lookAt(eye, center, up),
            .projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f)
        }; ubo.projection[1][1] *= -1; frame.uniform->write(&ubo, sizeof(ubo), true);

        auto positions = std::vector<glm::mat4x4>();
        positions.reserve(scene.triangle_positions.size());

        for (const auto& position : scene.triangle_positions)
            positions.push_back(glm::translate(glm::mat4x4(1.f), position));

        frame.storage->write(positions.data(), positions.size() * sizeof(glm::mat4x4), true);

    }

    void Engine::record_draw_commands (uint32_t index, Scene& scene) {

        auto& frame = swapchain->get_frames().at(index);
        auto& command_buffer = frame.commands;

        auto begin_info = vk::CommandBufferBeginInfo();
        try {
            command_buffer.begin(begin_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to begin command record");
        }

        auto clear_color = vk::ClearValue { std::array {1.f, 1.f, 1.f, 1.f} };

        auto renderpass_info = vk::RenderPassBeginInfo {
            .renderPass = renderpass,
            .framebuffer = frame.buffer,
            .renderArea = {{0, 0}, swapchain->get_extent()},
            .clearValueCount = 1,
            .pClearValues = &clear_color
        };

        command_buffer.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &frame.descriptor_set, 0, nullptr);

        auto offsets = std::array<vk::DeviceSize, 1> {}; 
        command_buffer.bindVertexBuffers(0, 1, &asset->vertex_buffer->get_handle(), offsets.data());

        auto viewport = vk::Viewport {
            .width = static_cast<float>(swapchain->get_extent().width),
            .height = static_cast<float>(swapchain->get_extent().height),       
            .minDepth = 0.f,
            .maxDepth = 1.f
        };

        command_buffer.setViewport(0, 1, &viewport);

        auto scissor = vk::Rect2D {
            .extent = swapchain->get_extent()
        };

        command_buffer.setScissor(0, 1, &scissor);

        prepare_frame(index, scene);
        auto vertex_count = to_u32(asset->vertices.size());
        auto instance_count = to_u32(scene.triangle_positions.size());
        command_buffer.draw(vertex_count,instance_count, 0, 0);

        if (is_imgui_enabled) imgui->draw(command_buffer, on_render);
        command_buffer.endRenderPass();

        try {
            command_buffer.end();
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to record command buffer");
        }

    }

    void Engine::draw (Scene& scene) {

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        const auto& frame = swapchain->get_frames().at(frame_number);

        auto wait_result = device->get_handle().waitForFences(frame.in_flight, VK_TRUE, timeout);
        device->get_handle().resetFences(frame.in_flight);

        auto image_result = device->get_handle().acquireNextImageKHR(swapchain->get_handle(), timeout, frame.image_available);
            
        frame.commands.reset();

        record_draw_commands(frame_number, scene);
        
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        auto submit_info = vk::SubmitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.image_available,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.commands,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &frame.render_finished
        };

        try {
            graphics_queue.submit(submit_info, frame.in_flight);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to submit draw command buffer")
        }

        auto present_info = vk::PresentInfoKHR {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.render_finished,
            .swapchainCount = 1,
            .pSwapchains = &swapchain->get_handle(),
            .pImageIndices = &image_result.value
        };

        auto present_result = present_queue.presentKHR(present_info);

        if (present_result == vk::Result::eErrorOutOfDateKHR 
            || present_result == vk::Result::eSuboptimalKHR 
            || is_framebuffer_resized) { remake_swapchain(); return; };

        frame_number = (frame_number + 1) % max_frames_in_flight;
    }

}