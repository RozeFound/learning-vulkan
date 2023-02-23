#include <limits>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "device.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "utils.hpp"

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

        device = Device(window);

        if constexpr (debug) {
            dldi = vk::DispatchLoaderDynamic(device.get_instance(), vkGetInstanceProcAddr);
            debug_messenger = make_debug_messenger(device.get_instance(), dldi);
        }

        auto indices = get_queue_family_indices(device.get_gpu(), device.get_surface());
        graphics_queue = device.get_handle().getQueue(indices.graphics_family.value(), 0);
        present_queue = device.get_handle().getQueue(indices.present_family.value(), 0);

        prepare();

    }

    Engine::~Engine ( ) {

        device.get_handle().waitIdle();

        if (is_imgui_enabled) imgui.destroy();

        LOG_INFO("Destroying Engine...");

        if (debug) device.get_instance().destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        LOG_INFO("Destroying Command Pool");
        device.get_handle().destroyCommandPool(command_pool);
        LOG_INFO("Destroying Descriptor Pool");
        device.get_handle().destroyDescriptorPool(descriptor_pool);

        pipeline.destroy();
        swapchain.destroy();

        delete asset;

    }

    void Engine::prepare ( ) {

        make_command_pool();

        pipeline = PipeLine(device.get_handle(), SwapChain::query_format(device.get_gpu(), device.get_surface()));

        swapchain = SwapChain(device, pipeline.get_renderpass());
        max_frames_in_flight = swapchain.get_frames().size();
        make_descriptor_pool();

        swapchain.make_commandbuffers(command_pool);
        swapchain.make_descriptor_sets(descriptor_pool, pipeline.get_descriptor_set_layout());

        if (is_imgui_enabled) imgui = ImGUI(device, swapchain, pipeline);

        frame_number = 0;

        asset = new Mesh(device);

    }

    void Engine::remake_swapchain ( ) {

        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(const_cast<GLFWwindow*>(device.get_window()), &width, &height);
            glfwWaitEvents();
        }

        if (width == swapchain.get_extent().width 
            && height == swapchain.get_extent().height)
            return;

        device.get_handle().waitIdle();

        swapchain.create_handle();

        frame_number = 0;
        is_framebuffer_resized = false;

    }

    void Engine::make_command_pool ( ) {

        auto indices = get_queue_family_indices(device.get_gpu(), device.get_surface());

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = indices.graphics_family.value()
        };

        try {
            command_pool = device.get_handle().createCommandPool(create_info);
            LOG_INFO("Successfully created Command Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Command Pool");
        }

    }

    void Engine::make_descriptor_pool ( ) {

        auto pool_size = vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = max_frames_in_flight
		};

        auto create_info = vk::DescriptorPoolCreateInfo {
            .flags = vk::DescriptorPoolCreateFlags(),
            .maxSets = max_frames_in_flight,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size
        };

        try {
            descriptor_pool = device.get_handle().createDescriptorPool(create_info);
            LOG_INFO("Successfully created Descriptor Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Descriptor Pool");
        }

    }

    void Engine::prepare_frame (uint32_t index) {

        auto& frame = swapchain.get_frames().at(index);

        glm::vec3 eye = { 2.0f, 2.0f, -1.0f };
        glm::vec3 center = { 0.0f, 0.0f, 0.0f };
        glm::vec3 up = { 0.0f, 0.0f, -1.0f };

        auto aspect = static_cast<float>(swapchain.get_extent().width) / static_cast<float>(swapchain.get_extent().height);

        auto ubo = UniformBufferObject {
            .view = glm::lookAt(eye, center, up),
            .projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f)
        }; ubo.projection[1][1] *= -1; frame.uniform_buffer.write(&ubo);

       frame.commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.get_pipeline_layout(), 0, 1, &frame.descriptor_set,0, nullptr);

    }

    void Engine::record_draw_commands (uint32_t index, Scene& scene) {

        auto& command_buffer = swapchain.get_frames().at(index).commands;

        auto begin_info = vk::CommandBufferBeginInfo();
        try {
            command_buffer.begin(begin_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to begin command record");
        }

        auto clear_color = vk::ClearValue { std::array {1.f, 1.f, 1.f, 1.f} };

        auto renderpass_info = vk::RenderPassBeginInfo {
            .renderPass = pipeline.get_renderpass(),
            .framebuffer = swapchain.get_frames().at(index).buffer,
            .renderArea = {{0, 0}, swapchain.get_extent()},
            .clearValueCount = 1,
            .pClearValues = &clear_color
        };

        command_buffer.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get_handle());

        auto offsets = std::array<vk::DeviceSize, 1> {}; 
        command_buffer.bindVertexBuffers(0, 1, &asset->vertex_buffer.get_handle(), offsets.data());


        auto viewport = vk::Viewport {
            .width = static_cast<float>(swapchain.get_extent().width),
            .height = static_cast<float>(swapchain.get_extent().height),       
            .minDepth = 0.f,
            .maxDepth = 1.f
        };

        command_buffer.setViewport(0, 1, &viewport);

        auto scissor = vk::Rect2D {
            .extent = swapchain.get_extent()
        };

        command_buffer.setScissor(0, 1, &scissor);

        prepare_frame(index);

        for (const auto& position : scene.triangle_positions) {

            auto matrix = glm::translate(glm::mat4x4(1.f), position);

            command_buffer.pushConstants(pipeline.get_pipeline_layout(), 
                vk::ShaderStageFlagBits::eVertex, 0, sizeof(matrix), &matrix);

            command_buffer.draw(to_u32(asset->vertices.size()), 1, 0, 0);

        }

        imgui.draw(command_buffer, on_render);
        command_buffer.endRenderPass();

        try {
            command_buffer.end();
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to record command buffer");
        }

    }

    void Engine::draw (Scene& scene) {

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        const auto& frame = swapchain.get_frames().at(frame_number);

        auto wait_result = device.get_handle().waitForFences(frame.in_flight, VK_TRUE, timeout);

        auto image_result = device.get_handle().acquireNextImageKHR(swapchain.get_handle(), timeout, frame.image_available);
    
        if (image_result.result == vk::Result::eErrorOutOfDateKHR)
            { remake_swapchain(); return; }

        device.get_handle().resetFences(frame.in_flight);

        frame.commands.reset();

        record_draw_commands(image_result.value, scene);
        
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
            .pSwapchains = &swapchain.get_handle(),
            .pImageIndices = &image_result.value
        };

        auto present_result = present_queue.presentKHR(present_info);

        if (present_result == vk::Result::eErrorOutOfDateKHR 
            || present_result == vk::Result::eSuboptimalKHR || is_framebuffer_resized)
            { remake_swapchain(); return; };

        frame_number = (frame_number + 1) % max_frames_in_flight;
    }

}