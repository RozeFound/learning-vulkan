#pragma once

#include <optional>

#include "transient.hpp"

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

    QueueFamilyIndices get_queue_family_indices (const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);

    vk::UniqueSemaphore make_semaphore (const vk::Device& device);
    vk::UniqueFence make_fence (const vk::Device& device);

    uint32_t to_u32 (std::size_t value);
}