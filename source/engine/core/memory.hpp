#pragma once

#include <memory>

#include <vk_mem_alloc.h>

#include "device.hpp"

namespace engine {

    void copy_buffer (const vk::Buffer& source, const vk::Buffer& destination, std::size_t size);
    uint32_t get_memory_index (vk::MemoryRequirements, vk::MemoryPropertyFlags);

    void insert_image_memory_barrier (const vk::CommandBuffer& command_buffer, const vk::Image& image,
        vk::ImageAspectFlags aspect_flags, const std::array<vk::PipelineStageFlags, 2> stages, 
        const std::array<vk::AccessFlags, 2> access_flags, const std::array<vk::ImageLayout, 2> layouts);

    class BasicBuffer {

        vk::UniqueBuffer handle;
        vk::UniqueDeviceMemory memory;
        void* data_location = nullptr;

        std::shared_ptr<Device> device = Device::get();

        std::size_t size;

        bool persistent;
        bool device_local;

        public:

        BasicBuffer (std::size_t size, vk::BufferUsageFlags usage, bool persistent = false, bool device_local = false);
        ~BasicBuffer ( );

        void write (const auto& data, std::size_t size = 0, std::ptrdiff_t offset = 0) {

            if (!size) size = get_size();

            if (device_local) {

                auto staging = BasicBuffer(size, vk::BufferUsageFlagBits::eTransferSrc);

                staging.write(data, size, offset);

                copy_buffer(staging.get_handle(), handle.get(), size);

            } else {

                if (!persistent) data_location = device->get_handle().mapMemory(memory.get(), 0, size);

                auto location = static_cast<std::byte*>(data_location) + offset;
                std::memcpy(location, data, size);

                if (!persistent) device->get_handle().unmapMemory(memory.get());

            }

        };

        constexpr const vk::Buffer& get_handle ( ) const { return handle.get(); }
        constexpr const vk::DeviceMemory& get_memory ( ) const { return memory.get(); }
        constexpr const std::size_t get_size ( ) const { return size; }

    };

    class VMABuffer {

        vk::Buffer handle;
        VmaAllocation allocation;
        VmaAllocationInfo alloc_info;

        bool persistent;
        bool device_local;

        std::size_t size;

        public:

        VMABuffer (std::size_t size, vk::BufferUsageFlags usage, bool persistent = false, bool device_local = false);
        ~VMABuffer ( );

        VMABuffer (const VMABuffer&) = delete;
        VMABuffer (const VMABuffer&&) = delete;

        void write (const auto& data, std::size_t size = 0, std::ptrdiff_t offset = 0) {

            if (!size) size = get_size();

            if (device_local) {

                auto staging = VMABuffer(size, vk::BufferUsageFlagBits::eTransferSrc);

                staging.write(data, size, offset);

                copy_buffer(staging.get_handle(), handle, size);

            } else {

                void* location;
                if (persistent) location = alloc_info.pMappedData;
                else vmaMapMemory(Device::get()->get_allocator(), allocation, &location);
                
                location = static_cast<std::byte*>(location) + offset;
                std::memcpy(location, data, size);

                if (!persistent) vmaUnmapMemory(Device::get()->get_allocator(), allocation);

            }

        }

        constexpr const vk::Buffer& get_handle ( ) const { return handle; }
        constexpr const std::size_t get_size ( ) const { return size; }

    };

    typedef VMABuffer Buffer;

}