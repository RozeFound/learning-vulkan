#pragma once

#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;
	};

    QueueFamilyIndices get_queue_family_indices (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

    std::optional<vk::Semaphore> make_semaphore (vk::Device& device);
    std::optional<vk::Fence> make_fence (vk::Device& device);

}