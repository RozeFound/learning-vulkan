#include <cstddef>
#include <limits>

#include "utils.hpp"
#include "swapchain.hpp"
#include "logging.hpp"

namespace engine {

    SwapChain::SwapChain (vk::PhysicalDevice& physical_device, vk::Device& device, vk::SurfaceKHR& surface, GLFWwindow* window) 
        : physical_device(physical_device), device(device), surface(surface), window(window) {

        query_swapchain_info();

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
            .imageExtent = get_extent( ),
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

    void SwapChain::query_swapchain_info ( ) {

        capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
        auto formats =  physical_device.getSurfaceFormatsKHR(surface);

        for (const auto& format : formats)
			if (format.format == vk::Format::eB8G8R8A8Unorm&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				{ this->format = format; break; } else this->format = formats.at(0);

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
            extent = capabilities.currentExtent;
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            auto min_width = std::max(capabilities.minImageExtent.width, static_cast<uint32_t>(width));
            auto min_height = std::max(capabilities.minImageExtent.height, static_cast<uint32_t>(height));

            extent = vk::Extent2D { 
                .width = std::min(min_width, capabilities.maxImageExtent.width),
                .height = std::min(min_height, capabilities.maxImageExtent.height)
            };
        }

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

            frames.at(i).image_available = make_semaphore(device);
            frames.at(i).render_finished = make_semaphore(device);
            frames.at(i).in_flight = make_fence(device);
		};

        LOG_INFO("Created ImageView's for SwapChain");

    }

}