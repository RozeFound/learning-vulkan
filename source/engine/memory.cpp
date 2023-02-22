#include <limits>

#include "memory.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    uint32_t get_memory_index (vk::PhysicalDevice& physical_device, vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags) {

        auto properties = physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

            bool supported = requirements.memoryTypeBits & (1 << i) ? true : false;
            bool sufficient = (properties.memoryTypes.at(i).propertyFlags & flags) == flags ? true : false;

            if (supported && sufficient) return i;

        }

        return std::numeric_limits<uint32_t>::max();

    }

    Buffer create_buffer (vk::PhysicalDevice& physical_device, vk::Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags flags) {

        auto create_info = vk::BufferCreateInfo {
            .flags = vk::BufferCreateFlags(),
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        try {
            auto buffer = device.createBuffer(create_info);     
            auto requirements = device.getBufferMemoryRequirements(buffer);
            auto index = get_memory_index(physical_device, requirements, flags);

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = index
            };

            auto memory = device.allocateMemory(allocate_info);
            device.bindBufferMemory(buffer, memory, 0);

            return {buffer, memory};
        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create buffer");
            return {nullptr, nullptr};
        }
        
    }

    void copy_buffer (vk::PhysicalDevice& physical_device, vk::Device& device, vk::SurfaceKHR& surface, vk::Buffer source, vk::Buffer destination, vk::DeviceSize size) {

        auto indices = get_queue_family_indices(physical_device, surface);
        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };
        auto command_pool = device.createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto command_buffer = device.allocateCommandBuffers(allocate_info).at(0);

        auto begin_info = vk::CommandBufferBeginInfo {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        command_buffer.begin(begin_info);

        auto copy_region = vk::BufferCopy {
            .size = size
        };

        command_buffer.copyBuffer(source, destination, 1, &copy_region);

        command_buffer.end();

        auto submit_info = vk::SubmitInfo {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer
        };

        auto transit_queue = device.getQueue(indices.transfer_family.value(), 0);
        transit_queue.submit(submit_info);

        transit_queue.waitIdle();
        device.destroyCommandPool(command_pool);

    }

}