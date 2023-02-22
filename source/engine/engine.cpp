#include <limits>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "instance.hpp"
#include "device.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    Engine::Engine (GLFWwindow* window) : window(window) {

        LOG_INFO("Creating Engine instance...");

        make_instance();
        make_device();
        prepare();

    }

    Engine::~Engine ( ) {

        device.waitIdle();

        if (is_imgui_enabled) imgui.destroy();

        LOG_INFO("Destroying Engine...");

        if (debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        LOG_INFO("Destroying Command Pool");
        device.destroyCommandPool(command_pool);

        pipeline.destroy();
        swapchain.destroy();

        delete asset;

        LOG_INFO("Destroying Device");
        device.destroy();

        LOG_INFO("Destroying Surface");
        instance.destroySurfaceKHR(surface);
        LOG_INFO("Destroying Instance");
        instance.destroy();

    }

    void Engine::make_instance ( ) {


        if constexpr (debug) {

            auto version = vk::enumerateInstanceVersion();

            auto major = VK_API_VERSION_MAJOR(version);
            auto minor = VK_API_VERSION_MINOR(version);
            auto patch = VK_API_VERSION_PATCH(version);

            LOG_INFO("System support up to Vulkan {}.{}.{}", major, minor, patch);

        }

        instance = create_instance();

        if constexpr (debug) {
            dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
            debug_messenger = make_debug_messenger(instance, dldi);
        }

        VkSurfaceKHR c_surface;

        if (glfwCreateWindowSurface(instance, window, nullptr, &c_surface) != VK_SUCCESS)
            LOG_ERROR("Cannot abstract GLFW surface for Vulkan");

        surface = c_surface;

    }

    void Engine::make_device ( ) {

        physical_device = get_physical_device(instance);
        device = create_logical_device(physical_device, surface);

        auto indices = get_queue_family_indices(physical_device, surface);
        graphics_queue = device.getQueue(indices.graphics_family.value(), 0);
        present_queue = device.getQueue(indices.present_family.value(), 0);

    }

    void Engine::prepare ( ) {

        make_command_pool();

        pipeline = PipeLine(device, SwapChain::query_format(physical_device, surface));
        swapchain = SwapChain(physical_device, device, surface, pipeline.get_renderpass(), command_pool, window);
        if (is_imgui_enabled) imgui = ImGUI(physical_device, device, instance, surface, swapchain, pipeline, window);

        max_frames_in_flight = swapchain.get_frames().size();
        frame_number = 0;

        asset = new Mesh(physical_device, device, surface);

    }

    void Engine::remake_swapchain ( ) {

        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        if (width == swapchain.get_extent().width 
            && height == swapchain.get_extent().height)
            return;

        device.waitIdle();

        swapchain.create_handle();

        frame_number = 0;
        is_framebuffer_resized = false;

    }

    void Engine::make_command_pool ( ) {

        auto indices = get_queue_family_indices(physical_device, surface);

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = indices.graphics_family.value()
        };

        try {
            command_pool = device.createCommandPool(create_info);
            LOG_INFO("Successfully created Command Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Command Pool");
        }

    }

    void Engine::record_draw_commands (uint32_t index, Scene& scene) {

        const auto& command_buffer = swapchain.get_frames().at(index).commands;

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
        command_buffer.bindVertexBuffers(0, 1, &asset->vertex_buffer.buffer, offsets.data());

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

        for (const auto& position : scene.triangle_positions) {

            auto matrix = glm::translate(glm::mat4x4(1.f), position);

            command_buffer.pushConstants(pipeline.get_layout(), 
                vk::ShaderStageFlagBits::eVertex, 0, sizeof(matrix), &matrix);

            command_buffer.draw(to_u32(asset->vertices.size()), 1, 0, 0);

        }

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

        auto wait_result = device.waitForFences(frame.in_flight, VK_TRUE, timeout);

        auto image_result = device.acquireNextImageKHR(swapchain.get_handle(), timeout, frame.image_available);
    
        if (image_result.result == vk::Result::eErrorOutOfDateKHR)
            { remake_swapchain(); return; }

        device.resetFences(frame.in_flight);

        frame.commands.reset();

        record_draw_commands(image_result.value, scene);
        
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        auto command_buffers = std::vector { frame.commands };

        if (is_imgui_enabled) {
            auto& imgui_command_buffer = imgui.get_commands(image_result.value, on_render);
            command_buffers.push_back(imgui_command_buffer);
        }

        auto submit_info = vk::SubmitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.image_available,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
            .pCommandBuffers = command_buffers.data(),
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