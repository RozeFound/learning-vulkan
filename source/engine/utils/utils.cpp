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

            if (queue_flags & vk::QueueFlagBits::eCompute
                && queue_flags & ~vk::QueueFlagBits::eGraphics) 
                indices.compute_family = i;

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

    vk::SampleCountFlagBits get_max_sample_count (const vk::PhysicalDevice& physical_device) {

        auto properties = physical_device.getProperties();

        auto color_samples_count = properties.limits.framebufferColorSampleCounts;
        auto depth_samples_count = properties.limits.framebufferDepthSampleCounts;

        auto sample_counts = color_samples_count & depth_samples_count;

        using enum vk::SampleCountFlagBits;

        auto sample_count = e1;

        if (sample_counts & e64) sample_count = e64;
        else if (sample_counts & e32) sample_count = e32;
        else if (sample_counts & e16) sample_count = e16;
        else if (sample_counts & e8) sample_count = e8;
        else if (sample_counts & e4) sample_count = e4;
        else if (sample_counts & e2) sample_count = e2;

        return sample_count;

    }

}