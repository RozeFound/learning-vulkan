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

        if (!vsync_enabled) {
            for (auto& mode : modes) 
            if (mode == vk::PresentModeKHR::eMailbox)
                { present_mode = mode; break; }
        }

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
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eInherit,
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

        using enum vk::ImageUsageFlagBits;
        using enum vk::SampleCountFlagBits;

        auto format = device->get_format().format;
        auto sample_count = get_max_sample_count(device->get_gpu());

        depth_buffer = std::make_shared<Image>(extent.width, extent.height, Image::get_depth_format(), eDepthStencilAttachment, 1, sample_count);
        color_buffer = std::make_shared<Image>(extent.width, extent.height, format, eTransientAttachment | eColorAttachment, 1, sample_count);

        auto images = device->get_handle().getSwapchainImagesKHR(handle.get());
        frames.resize(images.size());

        for (size_t i = 0; i < images.size(); i++) {

			frames.at(i).image = images.at(i);
			frames.at(i).view = Image::create_view(images.at(i), format, vk::ImageAspectFlagBits::eColor);

            frames.at(i).depth_buffer = depth_buffer;
            frames.at(i).color_buffer = color_buffer;

            auto attachments = std::array { color_buffer->get_view(), frames.at(i).view.get(), depth_buffer->get_view() };

            auto create_info = vk::FramebufferCreateInfo {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = render_pass,
                .attachmentCount = to_u32(attachments.size()),
                .pAttachments = attachments.data(),
                .width = extent.width,
                .height = extent.height,
                .layers = 1
            };

            try {
                frames.at(i).buffer = device->get_handle().createFramebufferUnique(create_info);
            } catch (vk::SystemError err) {
                loge("Failed to create Framebuffer");
            }

		};

        logi("Created ImageView's for SwapChain");

        for (auto& frame : frames) {          
            frame.image_available = make_semaphore(device->get_handle());
            frame.render_finished = make_semaphore(device->get_handle());
            frame.in_flight = make_fence(device->get_handle());
        }

    }

    uint32_t SwapChain::acquire_image (uint32_t index) {

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        const auto& frame = frames.at(index);

        if (device->get_handle().waitForFences(frame.in_flight.get(), VK_TRUE, timeout) != vk::Result::eSuccess)
            logw("Something goes wrong when waiting on fences");
        device->get_handle().resetFences(frame.in_flight.get());

        auto image_result = device->get_handle().acquireNextImageKHR(handle.get(), timeout, frame.image_available.get());

        if (image_result.result == vk::Result::eErrorOutOfDateKHR) resize_if_needed();

        return image_result.value;

    }

    bool SwapChain::present_image(uint32_t index) {

        const auto& frame = frames.at(index);

        auto present_info = vk::PresentInfoKHR {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.render_finished.get(),
            .swapchainCount = 1,
            .pSwapchains = &handle.get(),
            .pImageIndices = &frame.index
        };

        try {
            auto result = queue.presentKHR(present_info);     
        } catch (vk::OutOfDateKHRError e) {
            resize_if_needed();
            return false;
        }

        return true;

    }

    bool SwapChain::resize_if_needed ( ) {
        
        auto new_extent = device->get_extent();

        if (new_extent == extent) return false;
        device->get_handle().waitIdle();
        create_handle();

        return true;

    }

}