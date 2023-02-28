#include "utils.hpp"
#include "logging.hpp"

namespace engine {

    TransientBuffer::TransientBuffer (const vk::Device& device, QueueFamilyIndices indices) : device(device) {

        queue = device.getQueue(indices.transfer_family.value(), 0);

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };

        pool = device.createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        buffer = device.allocateCommandBuffers(allocate_info).at(0);

    }

    TransientBuffer::~TransientBuffer ( ) {

        queue.waitIdle();
        device.destroyCommandPool(pool);

    }

    vk::CommandBuffer& TransientBuffer::get ( ) {

        auto begin_info = vk::CommandBufferBeginInfo {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        buffer.begin(begin_info);

        return buffer;

    }

    void TransientBuffer::submit ( ) {

        buffer.end();

        auto submit_info = vk::SubmitInfo {
            .commandBufferCount = 1,
            .pCommandBuffers = &buffer
        };

        queue.submit(submit_info);

    }

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

    vk::Semaphore make_semaphore (const vk::Device& device) {

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

    vk::Fence make_fence (const vk::Device& device) {

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

    uint32_t to_u32 (std::size_t value) {
        return static_cast<uint32_t>(value);
    }

}