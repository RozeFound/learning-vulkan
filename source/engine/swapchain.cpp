#include <cstddef>
#include <limits>

#include "utils.hpp"
#include "swapchain.hpp"
#include "logging.hpp"

namespace engine {

    SwapChain::SwapChain (vk::PhysicalDevice& physical_device, vk::Device& device, vk::SurfaceKHR& surface, const vk::RenderPass& renderpass, vk::CommandPool& command_pool, GLFWwindow* window) 
        : physical_device(physical_device), device(device), surface(surface), renderpass(renderpass), command_pool(command_pool), window(window) {

        capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
        format = query_format(physical_device, surface);
        extent = query_extent(capabilities, window);

        create_swapchain();

    }

    void SwapChain::create_swapchain ( ) {

        auto modes = physical_device.getSurfacePresentModesKHR(surface);
        auto format = get_format();

        auto present_mode = vk::PresentModeKHR::eFifo;

        for (auto& mode : modes) 
            if (mode == vk::PresentModeKHR::eMailbox)
                { present_mode = mode; break; }


        auto create_info = vk::SwapchainCreateInfoKHR {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = surface, 
            .minImageCount = capabilities.minImageCount,
            .imageFormat = format.format,
            .imageColorSpace = format.colorSpace,
            .imageExtent = get_extent(),
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            // We'll set sharing mode and queue indices below //
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
        };

        auto indices = get_queue_family_indices(physical_device, surface);
        uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

        if (indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = vk::SharingMode::eConcurrent;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
        } else create_info.imageSharingMode = vk::SharingMode::eExclusive;

        try {
            handle = device.createSwapchainKHR(create_info);
            LOG_INFO("Successfully created SwapChain");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create SwapChain");
        }

        make_frames();

    }

    void SwapChain::destroy ( ) {

        LOG_INFO("Destroying Swapchain Frames");
        for (const auto& frame : frames) {
            device.destroyFramebuffer(frame.buffer);
            device.destroyImageView(frame.view);

            device.destroySemaphore(frame.image_available);
            device.destroySemaphore(frame.render_finished);
            device.destroyFence(frame.in_flight);
        }
            
        LOG_INFO("Destroying Swapchain");
        device.destroySwapchainKHR(handle);

    }

    void SwapChain::recreate ( ) {

        destroy();

        extent = query_extent(capabilities, window);

        create_swapchain();

    }

    vk::SurfaceFormatKHR SwapChain::query_format (vk::PhysicalDevice& physical_device, vk::SurfaceKHR& surface) {

        auto formats =  physical_device.getSurfaceFormatsKHR(surface);

        for (const auto& format : formats)
            if (format.format == vk::Format::eB8G8R8A8Unorm 
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return format;

        return formats.at(0);

    }

    vk::Extent2D SwapChain::query_extent (vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
            return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        auto min_width = std::max(capabilities.minImageExtent.width, static_cast<uint32_t>(width));
        auto min_height = std::max(capabilities.minImageExtent.height, static_cast<uint32_t>(height));

        return vk::Extent2D { 
            .width = std::min(min_width, capabilities.maxImageExtent.width),
            .height = std::min(min_height, capabilities.maxImageExtent.height)
        };

    }

    vk::Extent2D SwapChain::query_extent (vk::PhysicalDevice& physical_device, vk::SurfaceKHR& surface, GLFWwindow* window) {

            auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
            return query_extent(capabilities, window);

        }

    void SwapChain::make_frames ( ) {

        auto images = device.getSwapchainImagesKHR(handle);
        frames.resize(images.size());

        for (size_t i = 0; i < images.size(); i++) {

            auto subres_range = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

			auto create_info = vk::ImageViewCreateInfo {
                .flags = vk::ImageViewCreateFlags(),
                .image = images.at(i),
                .viewType = vk::ImageViewType::e2D,
                .format = format.format,
                .components = vk::ComponentMapping(),
                .subresourceRange = subres_range
            };

			frames.at(i).image = images.at(i);
			frames.at(i).view = device.createImageView(create_info);

		};

        LOG_INFO("Created ImageView's for SwapChain");

        for (auto& frame : frames) {
            frame.image_available = make_semaphore(device);
            frame.render_finished = make_semaphore(device);
            frame.in_flight = make_fence(device);
        }

        make_framebuffers();
        make_commandbuffers();

    }

    void SwapChain::make_framebuffers ( ) {

        for (auto& frame : frames) {

            auto attachments = std::vector { frame.view };

            auto create_info = vk::FramebufferCreateInfo {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = renderpass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = extent.width,
                .height = extent.height,
                .layers = 1
            };

            try {
                frame.buffer = device.createFramebuffer(create_info);
            } catch (vk::SystemError err) {
                LOG_ERROR("Failed to create Framebuffer");
            }

        }

        LOG_INFO("Created buffers for frames");

    }

    void SwapChain::make_commandbuffers ( ) {
        
        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(frames.size())
        };

        try {
            auto buffers = device.allocateCommandBuffers(allocate_info);
            for (std::size_t i = 0; i < frames.size(); i++)
                frames.at(i).commands = buffers.at(i);
            LOG_INFO("Allocated Command Buffers");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate Command Buffers");
        }

    }

}