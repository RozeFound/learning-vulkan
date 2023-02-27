#include <limits>

#include "memory.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    uint32_t get_memory_index (const vk::PhysicalDevice& physical_device, vk::MemoryRequirements requirements, vk::MemoryPropertyFlags flags) {

        auto properties = physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

            bool supported = requirements.memoryTypeBits & (1 << i);
            bool sufficient = (properties.memoryTypes.at(i).propertyFlags & flags) == flags;

            if (supported && sufficient) return i;

        }

        return std::numeric_limits<uint32_t>::max();

    }

    Buffer::Buffer (std::shared_ptr<Device> device, std::size_t size, vk::BufferUsageFlags usage, bool device_local) 
        : device(device), size(size), device_local(device_local) {

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
            handle = device->get_handle().createBuffer(create_info);     
            auto requirements = device->get_handle().getBufferMemoryRequirements(handle);
            auto index = get_memory_index(device->get_gpu(), requirements, flags);

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = index
            };

            memory = device->get_handle().allocateMemory(allocate_info);
            device->get_handle().bindBufferMemory(handle, memory, 0);  

        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create buffer");
        }

    }

    Buffer::~Buffer ( ) {

        if (!device_local && persistent_memory) 
            device->get_handle().unmapMemory(memory);

        device->get_handle().destroyBuffer(handle);
        device->get_handle().freeMemory(memory);

    }

    void copy_buffer (std::shared_ptr<Device> device, const vk::Buffer& source, vk::Buffer& destination, std::size_t size) {

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };
        auto command_pool = device->get_handle().createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto command_buffer = device->get_handle().allocateCommandBuffers(allocate_info).at(0);

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

        auto transit_queue = device->get_handle().getQueue(indices.transfer_family.value(), 0);
        transit_queue.submit(submit_info);

        transit_queue.waitIdle();
        device->get_handle().destroyCommandPool(command_pool);

    }

}