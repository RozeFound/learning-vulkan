#include "swapchain.hpp"
#include <cstddef>
#include "device.hpp"

namespace engine {

    std::optional<vk::SwapchainKHR> create_swapchain(engine::Engine& engine) {

        auto[physical_device, device] = engine.get_devices();
        auto surface = engine.get_surface();
        auto[width, height] = engine.get_dimensions(); 

        auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
        auto formats = physical_device.getSurfaceFormatsKHR(surface);
        auto modes = physical_device.getSurfacePresentModesKHR(surface);

        auto present_mode = vk::PresentModeKHR::eFifo;

        for (auto& mode : modes) 
            if (mode == vk::PresentModeKHR::eMailbox) {
                present_mode = mode; break; }

        auto min_width = std::max(capabilities.minImageExtent.width, static_cast<uint32_t>(width));
        auto min_height = std::max(capabilities.minImageExtent.height, static_cast<uint32_t>(height));

        auto extent = vk::Extent2D { 
            .width = std::min(min_width, capabilities.maxImageExtent.width),
            .height = std::min(min_height, capabilities.maxImageExtent.height)
        };

        auto create_info = vk::SwapchainCreateInfoKHR {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = surface, 
            .minImageCount = capabilities.minImageCount,
            .imageFormat = formats.at(0).format,
            .imageColorSpace = formats.at(0).colorSpace,
            .imageExtent = extent,
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
            auto result = device.createSwapchainKHR(create_info);
            LOG_INFO("Successfully created SwapChain");
            return result;
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create SwapChain");
            return std::nullopt;
        }

    }

}