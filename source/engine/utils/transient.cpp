#include "transient.hpp"

#include "../core/device.hpp"

#include "utils.hpp"

namespace engine {

        TransientBuffer::TransientBuffer ( ) {

        auto device = Device::get();

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        queue = device->get_handle().getQueue(indices.transfer_family.value(), 0);

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };

        pool = device->get_handle().createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        buffer = device->get_handle().allocateCommandBuffers(allocate_info).at(0);

    }

    TransientBuffer::~TransientBuffer ( ) {

        queue.waitIdle();
        Device::get()->get_handle().destroyCommandPool(pool);

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

}