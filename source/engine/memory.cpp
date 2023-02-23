#include <limits>

#include "memory.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    uint32_t get_memory_index (const vk::PhysicalDevice& physical_device, vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags) {

        auto properties = physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

            bool supported = requirements.memoryTypeBits & (1 << i) ? true : false;
            bool sufficient = (properties.memoryTypes.at(i).propertyFlags & flags) == flags ? true : false;

            if (supported && sufficient) return i;

        }

        return std::numeric_limits<uint32_t>::max();

    }

    Buffer::Buffer (Device& device, std::size_t size, vk::BufferUsageFlags usage, bool device_local) 
        : device(device), size(size), device_local(device_local) {

        using enum vk::MemoryPropertyFlagBits;
        using enum vk::BufferUsageFlagBits;

        if (device_local) {

            create_buffer(eTransferSrc, eHostVisible | eHostCoherent);  
            create_buffer(eTransferDst | usage, eDeviceLocal);
        
        } else {
            create_buffer(usage, eHostVisible | eHostCoherent);
            data_location = device.get_handle().mapMemory(memory, 0, size);
        }

    }

    void Buffer::destroy ( ) {

        if (device_local) {
            device.get_handle().destroyBuffer(staging_handle);
            device.get_handle().freeMemory(staging_memory);
        } else device.get_handle().unmapMemory(memory);

        device.get_handle().destroyBuffer(handle);
        device.get_handle().freeMemory(memory);

    }

    void Buffer::create_buffer (vk::BufferUsageFlags usage, vk::MemoryPropertyFlags flags) {

        auto create_info = vk::BufferCreateInfo {
            .flags = vk::BufferCreateFlags(),
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        try {
            auto buffer = device.get_handle().createBuffer(create_info);     
            auto requirements = device.get_handle().getBufferMemoryRequirements(buffer);
            auto index = get_memory_index(device.get_gpu(), requirements, flags);

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = index
            };

            auto memory = device.get_handle().allocateMemory(allocate_info);
            device.get_handle().bindBufferMemory(buffer, memory, 0);  

            if (usage & vk::BufferUsageFlagBits::eTransferSrc) {
                staging_handle = buffer; staging_memory = memory;
            } else { handle = buffer; this->memory = memory; };

        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create buffer");
        }

    }

    void Buffer::copy_buffer (vk::Buffer& source, vk::Buffer& destination) {

        auto indices = get_queue_family_indices(device.get_gpu(), device.get_surface());
        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };
        auto command_pool = device.get_handle().createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto command_buffer = device.get_handle().allocateCommandBuffers(allocate_info).at(0);

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

        auto transit_queue = device.get_handle().getQueue(indices.transfer_family.value(), 0);
        transit_queue.submit(submit_info);

        transit_queue.waitIdle();
        device.get_handle().destroyCommandPool(command_pool);

    }

}