#include "utils.hpp"
#include "logging.hpp"

namespace engine {

    QueueFamilyIndices get_queue_family_indices (vk::PhysicalDevice& device, vk::SurfaceKHR& surface) {

        QueueFamilyIndices indices;
        auto queue_family_properties = device.getQueueFamilyProperties();

        for (std::size_t i = 0; i < queue_family_properties.size(); i++) {

            auto queue_flags = queue_family_properties.at(i).queueFlags;
            
            if (queue_flags & vk::QueueFlagBits::eGraphics) indices.graphics_family = i;
            if (device.getSurfaceSupportKHR(i, surface)) indices.present_family = i;
            if(indices.graphics_family.has_value() && indices.present_family.has_value()) break;

        }

        return indices;

    }

    vk::Semaphore make_semaphore (vk::Device& device) {

        auto create_info = vk::SemaphoreCreateInfo {
            .flags = vk::SemaphoreCreateFlags()
        };

        try {
            return device.createSemaphore(create_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Semaphore");
            return nullptr;
        }

    }

    vk::Fence make_fence (vk::Device& device) {

        auto create_info = vk::FenceCreateInfo {
            .flags = vk::FenceCreateFlagBits::eSignaled
        };

        try {
            return device.createFence(create_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Fence");
            return nullptr;
        }

    }

}