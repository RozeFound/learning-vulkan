#pragma once

#include <functional>
#include <optional>
#include <chrono>

#include "transient.hpp"

namespace engine {

        class ScopedTimer {

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point;

        std::function<void(double)> callback;

        public:

		ScopedTimer (std::function<void(double)> callback) : callback(callback) {

			start_time_point = std::chrono::high_resolution_clock::now();

		}

		~ScopedTimer ( ) {

			auto end_time_point = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_point).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point).time_since_epoch().count();

            callback((end - start) * 0.001);

		}

	};

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