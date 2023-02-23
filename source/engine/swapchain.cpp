#include <cstddef>
#include <limits>

#include "utils.hpp"
#include "swapchain.hpp"
#include "logging.hpp"

namespace engine {

    SwapChain::SwapChain (Device& device, const vk::RenderPass& renderpass) : device(device), renderpass(renderpass) {

        capabilities = device.get_gpu().getSurfaceCapabilitiesKHR(device.get_surface());
        format = query_format(device.get_gpu(), device.get_surface());

        create_handle();

    }

    void SwapChain::create_handle ( ) {

        auto modes = device.get_gpu().getSurfacePresentModesKHR(device.get_surface());
        auto format = get_format();
        extent = query_extent(capabilities, device.get_window());

        auto present_mode = vk::PresentModeKHR::eFifo;

        for (auto& mode : modes) 
            if (mode == vk::PresentModeKHR::eMailbox)
                { present_mode = mode; break; }


        auto create_info = vk::SwapchainCreateInfoKHR {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = device.get_surface(), 
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
            .clipped = VK_TRUE
        };      

        auto indices = get_queue_family_indices(device.get_gpu(), device.get_surface());
        uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

        if (indices.graphics_family != indices.present_family) {
                create_info.imageSharingMode = vk::SharingMode::eConcurrent;
                create_info.queueFamilyIndexCount = 2;
                create_info.pQueueFamilyIndices = queue_family_indices;
        } else create_info.imageSharingMode = vk::SharingMode::eExclusive;

        try {
            if (handle) create_info.oldSwapchain = handle;
            auto result = device.get_handle().createSwapchainKHR(create_info);
            if (handle) destroy();
            handle = result;
            LOG_INFO("Successfully created SwapChain");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create SwapChain");
        }

        make_frames();

    }

    void SwapChain::destroy ( ) {

        LOG_INFO("Destroying Swapchain Frames");
        for (const auto& frame : frames) {
            device.get_handle().destroyFramebuffer(frame.buffer);
            device.get_handle().destroyImageView(frame.view);

            device.get_handle().destroySemaphore(frame.image_available);
            device.get_handle().destroySemaphore(frame.render_finished);
            device.get_handle().destroyFence(frame.in_flight);
        }
            
        LOG_INFO("Destroying Swapchain");
        device.get_handle().destroySwapchainKHR(handle);

    }

    vk::SurfaceFormatKHR SwapChain::query_format (const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface) {

        auto formats =  physical_device.getSurfaceFormatsKHR(surface);

        for (const auto& format : formats)
            if (format.format == vk::Format::eB8G8R8A8Unorm 
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return format;

        return formats.at(0);

    }

    vk::Extent2D SwapChain::query_extent (vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindow* window) {

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
            return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &width, &height);

        auto min_width = std::max(capabilities.minImageExtent.width, static_cast<uint32_t>(width));
        auto min_height = std::max(capabilities.minImageExtent.height, static_cast<uint32_t>(height));

        return vk::Extent2D { 
            .width = std::min(min_width, capabilities.maxImageExtent.width),
            .height = std::min(min_height, capabilities.maxImageExtent.height)
        };

    }

    vk::Extent2D SwapChain::query_extent (const vk::PhysicalDevice& physical_device, const vk::SurfaceKHR& surface, const GLFWwindow* window) {

            auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
            return query_extent(capabilities, window);

        }

    void SwapChain::make_frames ( ) {

        auto images = device.get_handle().getSwapchainImagesKHR(handle);
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
			frames.at(i).view = device.get_handle().createImageView(create_info);

		};

        LOG_INFO("Created ImageView's for SwapChain");

        for (auto& frame : frames) {          
            frame.image_available = make_semaphore(device.get_handle());
            frame.render_finished = make_semaphore(device.get_handle());
            frame.in_flight = make_fence(device.get_handle());
        }

        make_framebuffers();

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
                frame.buffer = device.get_handle().createFramebuffer(create_info);
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
            .commandBufferCount = static_cast<uint32_t>(frames.size())
        };

        try {
            auto buffers = device.get_handle().allocateCommandBuffers(allocate_info);
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
            auto descriptor_sets = device.get_handle().allocateDescriptorSets(allocate_info);
            for (std::size_t i = 0; i < frames.size(); i++)
                frames.at(i).descriptor_set = descriptor_sets.at(i);
            LOG_INFO("Allocated DescriptorSet's");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate DescriptorSet's");
        }

        for (auto& frame : frames) {

            frame.uniform_buffer = Buffer(device, sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer);  

            auto buffer_info = vk::DescriptorBufferInfo {
                .buffer = frame.uniform_buffer.get_handle(),
                .offset = 0,
                .range = sizeof(UniformBufferObject)
            };

            auto write_info = vk::WriteDescriptorSet {
                .dstSet = frame.descriptor_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &buffer_info
            };

            device.get_handle().updateDescriptorSets(1, &write_info, 0, nullptr);

        }

    }

}