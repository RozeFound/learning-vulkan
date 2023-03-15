#include <limits>

#define VMA_IMPLEMENTATION

#include "memory.hpp"

#include "../utils/logging.hpp"
#include "../utils/utils.hpp"

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

    BasicBuffer::BasicBuffer (std::size_t size, vk::BufferUsageFlags usage, bool persistent, bool device_local) 
        : size(size), persistent(persistent), device_local(device_local) {

        auto flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        if (device_local) {
            usage |= vk::BufferUsageFlagBits::eTransferDst;
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
            loge("Failed to create buffer");
        }

        if (persistent) data_location = device->get_handle().mapMemory(memory.get(), 0, size);

    }

    BasicBuffer::~BasicBuffer ( ) {

        if (!persistent) device->get_handle().unmapMemory(memory.get());

    }

    VMABuffer::VMABuffer (std::size_t size, vk::BufferUsageFlags usage, bool persistent, bool device_local) 
        : size(size), persistent(persistent), device_local(device_local) {

        auto flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        if(device_local) {
            usage |= vk::BufferUsageFlagBits::eTransferDst;
            flags = (VmaAllocationCreateFlagBits)0;
        }

        auto create_info = vk::BufferCreateInfo {
            .flags = vk::BufferCreateFlags(),
            .size = size, 
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        auto allocation_info = VmaAllocationCreateInfo {
            .flags = flags,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        if (!device_local) {
            allocation_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocation_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        if (persistent) allocation_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(Device::get()->get_allocator(), reinterpret_cast<VkBufferCreateInfo*>(&create_info),
            &allocation_info, reinterpret_cast<VkBuffer*>(&handle), &allocation, &alloc_info);

    }

    VMABuffer::~VMABuffer ( ) {

        vmaDestroyBuffer(Device::get()->get_allocator(), VkBuffer(handle), allocation);

    }

    void copy_buffer (const vk::Buffer& source, const vk::Buffer& destination, std::size_t size) {

        auto device = Device::get();

        auto transient_buffer = TransientBuffer();
        auto command_buffer = transient_buffer.get();

        auto copy_region = vk::BufferCopy {
            .size = size
        };

        command_buffer.copyBuffer(source, destination, 1, &copy_region);

        transient_buffer.submit();

    }

    void insert_image_memory_barrier (const vk::CommandBuffer& command_buffer, 
        const vk::ImageMemoryBarrier& barrier, const std::array<vk::PipelineStageFlags, 2> stages) {

        command_buffer.pipelineBarrier(stages.at(0), stages.at(1),
            vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

    }

    void insert_image_memory_barrier (const vk::CommandBuffer& command_buffer, const vk::Image& image,
        vk::ImageAspectFlags aspect_flags, const std::array<vk::PipelineStageFlags, 2> stages, 
        const std::array<vk::AccessFlags, 2> access_flags, const std::array<vk::ImageLayout, 2> layouts, uint32_t mip_levels) {

        auto subres_range = vk::ImageSubresourceRange {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        auto barrier = vk::ImageMemoryBarrier {
            .srcAccessMask = access_flags.at(0),
            .dstAccessMask = access_flags.at(1),
            .oldLayout = layouts.at(0),
            .newLayout = layouts.at(1),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subres_range
        };

        return insert_image_memory_barrier(command_buffer, barrier, stages);

    }

}