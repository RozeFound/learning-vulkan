#include <limits>

#include "memory.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    uint32_t get_memory_index (vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags) {

        auto properties = Device::get()->get_gpu().getMemoryProperties();

        for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

            bool supported = requirements.memoryTypeBits & (1 << i);
            bool sufficient = (properties.memoryTypes.at(i).propertyFlags & flags) == flags;

            if (supported && sufficient) return i;

        }

        return std::numeric_limits<uint32_t>::max();

    }

    Buffer::Buffer (std::size_t size, vk::BufferUsageFlags usage, bool device_local) 
        : size(size), device_local(device_local) {

        auto flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        if (device_local) {
            usage = vk::BufferUsageFlagBits::eTransferDst | usage;
            flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        }

        auto create_info = vk::BufferCreateInfo {
            .flags = vk::BufferCreateFlags(),
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        try {
            
            handle = device->get_handle().createBufferUnique(create_info);

            auto requirements = device->get_handle().getBufferMemoryRequirements(handle.get());

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = get_memory_index(requirements, flags)
            };

            memory = device->get_handle().allocateMemoryUnique(allocate_info);
            device->get_handle().bindBufferMemory(handle.get(), memory.get(), 0);  
        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create buffer");
        }

    }

    Buffer::~Buffer ( ) {

        if (!device_local && persistent_memory) 
            device->get_handle().unmapMemory(memory.get());

    }

    void copy_buffer (const vk::Buffer& source, vk::Buffer& destination, std::size_t size) {

        auto device = Device::get();

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto transient_buffer = TransientBuffer(device->get_handle(), indices);
        auto command_buffer = transient_buffer.get();

        auto copy_region = vk::BufferCopy {
            .size = size
        };

        command_buffer.copyBuffer(source, destination, 1, &copy_region);

        transient_buffer.submit();

    }

}