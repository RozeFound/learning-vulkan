#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace engine {

    class Buffer {

        vk::Buffer handle;
        vk::DeviceMemory memory;
        void* data_location;

        vk::Buffer staging_handle;
        vk::DeviceMemory staging_memory;

        Device device;

        std::size_t size;

        bool device_local;

        void create_buffer (vk::BufferUsageFlags, vk::MemoryPropertyFlags);
        void copy_buffer (vk::Buffer& src, vk::Buffer& dst, std::size_t size);

        public:

        Buffer ( ) = default;
        Buffer (Device& device, std::size_t size, vk::BufferUsageFlags usage, bool device_local = false);

        void destroy ( ); 

        void write (auto data, std::size_t size = 0) {

            if (!size) size = get_size();

            if (device_local) {

                void* mapped_memory = device.get_handle().mapMemory(staging_memory, 0, size);
                std::memcpy(mapped_memory, data, size);
                device.get_handle().unmapMemory(staging_memory);

                copy_buffer(staging_handle, handle, size);

            } else std::memcpy(data_location, data, size);

        };

        constexpr const vk::Buffer& get_handle ( ) const { return handle; };
        constexpr const std::size_t get_size ( ) const { return size; };

    };

    uint32_t get_memory_index (vk::PhysicalDevice&, vk::MemoryRequirements, vk::MemoryPropertyFlags);

}