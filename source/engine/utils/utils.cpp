#include <stdexcept>

#include "utils.hpp"

namespace engine {

    QueueFamilyIndices get_queue_family_indices (const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {

        QueueFamilyIndices indices;
        auto queue_family_properties = device.getQueueFamilyProperties();

        for (std::size_t i = 0; i < queue_family_properties.size(); i++) {

            auto queue_flags = queue_family_properties.at(i).queueFlags;
            
            if (queue_flags & vk::QueueFlagBits::eGraphics) indices.graphics_family = i;
            if (device.getSurfaceSupportKHR(i, surface)) indices.present_family = i;

            if (queue_flags & vk::QueueFlagBits::eTransfer 
                && queue_flags & ~vk::QueueFlagBits::eGraphics) 
                indices.transfer_family = i;

            if(indices.is_complete()) break;

        }

        return indices;

    }

    vk::UniqueSemaphore make_semaphore (const vk::Device& device) {

        auto create_info = vk::SemaphoreCreateInfo {
            .flags = vk::SemaphoreCreateFlags()
        };

        try {
            return device.createSemaphoreUnique(create_info);
        } catch (vk::SystemError err) {
            throw std::runtime_error("Failed to create Semaphore");
        }

    }

    vk::UniqueFence make_fence (const vk::Device& device) {

        auto create_info = vk::FenceCreateInfo {
            .flags = vk::FenceCreateFlagBits::eSignaled
        };

        try {
            return device.createFenceUnique(create_info);
        } catch (vk::SystemError err) {
            throw std::runtime_error("Failed to create Fence");
        }

    }

    uint32_t to_u32 (std::size_t value) {
        return static_cast<uint32_t>(value);
    }

}