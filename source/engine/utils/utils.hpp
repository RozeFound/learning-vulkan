#pragma once

#include <optional>

#include "transient.hpp"

namespace engine {

    template <std::size_t size> struct fixed_string {
            
        char buffer[size] { };

        constexpr fixed_string (const char* string) {
            for (std::size_t i = 0; i != size; i++) buffer[i] = string[i];
        }
        constexpr operator const char* ( ) const { return buffer; }
        constexpr bool operator<=> (const fixed_string&) const = default;
    };

    template <std::size_t size> fixed_string (const char (&)[size]) -> fixed_string<size>;

    struct QueueFamilyIndices {
        
        std::optional<uint32_t> transfer_family;
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;
        std::optional<uint32_t> compute_family;

        constexpr bool is_complete ( ) {
            if (transfer_family.has_value()
                && graphics_family.has_value()
                && present_family.has_value()
                && compute_family.has_value())
                return true;
            return false;
        }
        
	};

    QueueFamilyIndices get_queue_family_indices (const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);

    vk::UniqueSemaphore make_semaphore (const vk::Device& device);
    vk::UniqueFence make_fence (const vk::Device& device);

    vk::SampleCountFlagBits get_max_sample_count (const vk::PhysicalDevice& physical_device);

    constexpr uint32_t to_u32 (std::size_t value) { return static_cast<uint32_t>(value); }

}