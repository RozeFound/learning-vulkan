#include <cstddef>
#include <limits>
#include <memory>

#include "swapchain.hpp"

#include "image.hpp"
#include "shaders.hpp"

#include "../utils/utils.hpp"
#include "../utils/logging.hpp"

namespace engine {

    void SwapChain::create_handle ( ) {

        auto capabilities = device->get_gpu().getSurfaceCapabilitiesKHR(device->get_surface());
        auto modes = device->get_gpu().getSurfacePresentModesKHR(device->get_surface());
        extent = device->get_extent();

        auto present_mode = vk::PresentModeKHR::eFifo;

        for (auto& mode : modes) 
            if (mode == vk::PresentModeKHR::eMailbox)
                { present_mode = mode; break; }

        auto create_info = vk::SwapchainCreateInfoKHR {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = device->get_surface(), 
            .minImageCount = capabilities.minImageCount + 1,
            .imageFormat = device->get_format().format,
            .imageColorSpace = device->get_format().colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            // We'll set sharing mode and queue indices below //
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = handle.get()
        };      

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

        if (indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = vk::SharingMode::eConcurrent;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
        } else create_info.imageSharingMode = vk::SharingMode::eExclusive;

        try {
            handle = device->get_handle().createSwapchainKHRUnique(create_info);
            logi("Successfully created SwapChain");
        } catch (vk::SystemError err) {
            loge("Failed to create SwapChain");
        }

        make_frames();

    }

    void SwapChain::make_frames ( ) {

        depth_buffer = std::make_shared<DepthImage>(extent.width, extent.height);
        color_buffer = std::make_shared<ColorImage>(extent.width, extent.height);

        auto images = device->get_handle().getSwapchainImagesKHR(handle.get());
        frames.resize(images.size());

        for (size_t i = 0; i < images.size(); i++) {

			frames.at(i).image = images.at(i);
			frames.at(i).view = create_view(images.at(i), device->get_format().format, vk::ImageAspectFlagBits::eColor);

            frames.at(i).depth_buffer = depth_buffer;
            frames.at(i).color_buffer = color_buffer;

		};

        logi("Created ImageView's for SwapChain");

        for (auto& frame : frames) {          
            frame.image_available = make_semaphore(device->get_handle());
            frame.render_finished = make_semaphore(device->get_handle());
            frame.in_flight = make_fence(device->get_handle());
        }

    }

    void SwapChain::make_commandbuffers (vk::CommandPool& command_pool) {
        
        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = to_u32(frames.size())
        };

        try {
            auto buffers = device->get_handle().allocateCommandBuffers(allocate_info);
            for (std::size_t i = 0; i < frames.size(); i++)
                frames.at(i).commands = buffers.at(i);
            logi("Allocated Command Buffers");
        } catch (vk::SystemError err) {
            loge("Failed to allocate Command Buffers");
        }

    }

}