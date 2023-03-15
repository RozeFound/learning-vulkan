#include "transient.hpp"

#include "../core/device.hpp"

#include "utils.hpp"
#include "logging.hpp"

namespace engine {

        TransientBuffer::TransientBuffer (bool graphics_capable) {

        auto device = Device::get();

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto queue_family_index = graphics_capable ? indices.graphics_family.value() : indices.transfer_family.value();
        queue = device->get_handle().getQueue(queue_family_index, 0);

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = queue_family_index
        };

        pool = device->get_handle().createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        buffer = device->get_handle().allocateCommandBuffers(allocate_info).at(0);
        submition_completed = make_fence(device->get_handle());

    }

    TransientBuffer::~TransientBuffer ( ) {

        auto device = Device::get();
        constexpr auto timeout = std::numeric_limits<uint64_t>::max();

        if(device->get_handle().waitForFences(1, &submition_completed.get(), VK_TRUE, timeout) != vk::Result::eSuccess)
            logw("Something goes wrong when waiting on fences");
        device->get_handle().destroyCommandPool(pool);

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

        Device::get()->get_handle().resetFences(submition_completed.get());
        queue.submit(submit_info, submition_completed.get());

    }

}