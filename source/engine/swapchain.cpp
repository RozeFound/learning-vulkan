#include <cstddef>
#include <limits>
#include <memory>

#include "image.hpp"
#include "utils.hpp"
#include "swapchain.hpp"
#include "logging.hpp"
#include "shaders.hpp"

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
            .oldSwapchain = handle
        };      

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

        if (indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = vk::SharingMode::eConcurrent;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
        } else create_info.imageSharingMode = vk::SharingMode::eExclusive;

        try {

            handle = device->get_handle().createSwapchainKHR(create_info);

            if (create_info.oldSwapchain) {

                LOG_INFO("Destroying Old Swapchain Frames");
                for (const auto& frame : frames) {
                    device->get_handle().destroyFramebuffer(frame.buffer);
                    device->get_handle().destroyImageView(frame.view);

                    device->get_handle().destroySemaphore(frame.image_available);
                    device->get_handle().destroySemaphore(frame.render_finished);
                    device->get_handle().destroyFence(frame.in_flight);
                }
                LOG_INFO("Destroying Old Swapchain");
                device->get_handle().destroySwapchainKHR(create_info.oldSwapchain);
                delete depth_buffer.release();

            }

            LOG_INFO("Successfully created SwapChain");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create SwapChain");
        }

        make_frames();

    }

    SwapChain::~SwapChain ( ) {

        LOG_INFO("Destroying Swapchain Frames");
        for (const auto& frame : frames) {
            device->get_handle().destroyFramebuffer(frame.buffer);
            device->get_handle().destroyImageView(frame.view);

            device->get_handle().destroySemaphore(frame.image_available);
            device->get_handle().destroySemaphore(frame.render_finished);
            device->get_handle().destroyFence(frame.in_flight);
        }
            
        LOG_INFO("Destroying Swapchain");
        device->get_handle().destroySwapchainKHR(handle);

    }

    void SwapChain::make_frames ( ) {

        auto images = device->get_handle().getSwapchainImagesKHR(handle);
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
                .format = device->get_format().format,
                .components = vk::ComponentMapping(),
                .subresourceRange = subres_range
            };

			frames.at(i).image = images.at(i);
			frames.at(i).view = device->get_handle().createImageView(create_info);

		};

        LOG_INFO("Created ImageView's for SwapChain");

        for (auto& frame : frames) {          
            frame.image_available = make_semaphore(device->get_handle());
            frame.render_finished = make_semaphore(device->get_handle());
            frame.in_flight = make_fence(device->get_handle());
        }

        make_framebuffers();

    }

    void SwapChain::make_framebuffers ( ) {

        depth_buffer = std::make_unique<DepthImage>(device, extent.width, extent.height);

        for (auto& frame : frames) {

            auto attachments = std::array { frame.view, depth_buffer->get_view() };

            auto create_info = vk::FramebufferCreateInfo {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = renderpass,
                .attachmentCount = to_u32(attachments.size()),
                .pAttachments = attachments.data(),
                .width = extent.width,
                .height = extent.height,
                .layers = 1
            };

            try {
                frame.buffer = device->get_handle().createFramebuffer(create_info);
            } catch (vk::SystemError err) {
                LOG_ERROR("Failed to create Framebuffer");
            }

        }

        LOG_INFO("Created buffers for frames");

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
            LOG_INFO("Allocated Command Buffers");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate Command Buffers");
        }

    }

    void SwapChain::make_descriptor_sets (vk::DescriptorPool& descriptor_pool, const vk::DescriptorSetLayout& layout) {

        auto layouts = std::vector(frames.size(), layout);

        auto allocate_info = vk::DescriptorSetAllocateInfo {
            .descriptorPool = descriptor_pool,
            .descriptorSetCount = to_u32(frames.size()),
            .pSetLayouts = layouts.data()
        };

        try {
            auto descriptor_sets = device->get_handle().allocateDescriptorSets(allocate_info);
            for (std::size_t i = 0; i < frames.size(); i++)
                frames.at(i).descriptor_set = descriptor_sets.at(i);
            LOG_INFO("Allocated DescriptorSet's");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate DescriptorSet's");
        }

        for (auto& frame : frames) {

            auto write_info = std::array<vk::WriteDescriptorSet, 1>();

            auto image_info = vk::DescriptorImageInfo {
                .sampler = frame.texture->get_sampler(),
                .imageView = frame.texture->get_view(),
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };

            write_info.at(0) = vk::WriteDescriptorSet {
                .dstSet = frame.descriptor_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &image_info
            };

            device->get_handle().updateDescriptorSets(to_u32(write_info.size()), write_info.data(), 0, nullptr);

        }

    }

}