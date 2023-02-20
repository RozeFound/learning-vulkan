#include <limits>
#include <vector>

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

        make_framebuffers();
        make_command_pool();
        make_commandbuffers();
        
    }

    Engine::~Engine ( ) {

        device.waitIdle();

        LOG_INFO("Destroying Engine...");

        if (debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        LOG_INFO("Destroying Command Pool");
        device.destroyCommandPool(command_pool);

        pipeline.destroy();
        swapchain.destroy();
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

        swapchain = SwapChain(physical_device, device, surface, window);
        pipeline = PipeLine(device, swapchain);

    }

    void Engine::make_framebuffers ( ) {

        for (auto& frame : swapchain.get_frames()) {

            auto attachments = std::vector { frame.view };

            auto create_info = vk::FramebufferCreateInfo {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = pipeline.get_renderpass(),
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = swapchain.get_extent().width,
                .height = swapchain.get_extent().height,
                .layers = 1
            };

            try {
                frame.buffer = device.createFramebuffer(create_info);
            } catch (vk::SystemError err) {
                LOG_ERROR("Failed to create Framebuffer");
            }

        }        

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

    void Engine::make_commandbuffers ( ) {
        
        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(swapchain.get_frames().size())
        };

        try {
            auto buffers = device.allocateCommandBuffers(allocate_info);
            for (std::size_t i = 0; i < swapchain.get_frames().size(); i++)
                swapchain.get_frames().at(i).commands = buffers.at(i);
            LOG_INFO("Allocated Command Buffer");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate Command Buffer");
        }

    }

    void Engine::record_draw_commands (uint32_t index) {

        const auto& command_buffer = swapchain.get_frames().at(index).commands;

        auto begin_info = vk::CommandBufferBeginInfo();
        try {
            command_buffer.begin(begin_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to begin command record");
        }

        auto clear_color = vk::ClearValue { std::array<float, 4>{1.f, 1.f, 1.f, 1.f} };

        auto renderpass_info = vk::RenderPassBeginInfo {
            .renderPass = pipeline.get_renderpass(),
            .framebuffer = swapchain.get_frames().at(index).buffer,
            .renderArea = {{0, 0}, swapchain.get_extent()},
            .clearValueCount = 1,
            .pClearValues = &clear_color
        };

        command_buffer.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get_handle());
        command_buffer.draw(3, 1, 0, 0);

        command_buffer.endRenderPass();

        try {
            command_buffer.end();
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to record command buffer");
        }

    }

    void Engine::draw ( ) {

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        const auto& frame = swapchain.get_frames().at(frame_number);

        auto wait_result = device.waitForFences(frame.in_flight, VK_TRUE, timeout);
        device.resetFences(frame.in_flight);

        auto image_result = device.acquireNextImageKHR(swapchain.get_handle(), timeout, frame.image_available);
        
        frame.commands.reset();

        record_draw_commands(frame_number);

        vk::Semaphore wait_semaphores[] = { frame.image_available };
        vk::Semaphore signal_semaphores[] = { frame.render_finished };
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        auto submit_info = vk::SubmitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = wait_semaphores,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.commands,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signal_semaphores
        };

        try {
            graphics_queue.submit(submit_info, frame.in_flight);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to submit draw command buffer")
        }

        auto present_info = vk::PresentInfoKHR {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = &swapchain.get_handle(),
            .pImageIndices = &image_result.value
        };

        auto present_result = present_queue.presentKHR(present_info);

        frame_number = (frame_number + 1) % max_frames_in_flight;
    }

}