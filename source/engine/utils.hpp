#pragma once

#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    struct QueueFamilyIndices {
        
        std::optional<uint32_t> transfer_family;
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

        constexpr bool is_complete ( ) {
            if (transfer_family.has_value()
                && graphics_family.has_value()
                && present_family.has_value())
                return true;
            return false;
        }
        
	};

    QueueFamilyIndices get_queue_family_indices (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

    vk::Semaphore make_semaphore (vk::Device& device);
    vk::Fence make_fence (vk::Device& device);

    uint32_t to_u32 (std::size_t value);

}