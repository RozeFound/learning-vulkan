#pragma once

#include "essentials.hpp"
#include "device.hpp"
#include <memory>

namespace engine {

    void copy_buffer (std::shared_ptr<Device> device, const vk::Buffer& source, vk::Buffer& destination, std::size_t size);
    uint32_t get_memory_index (const vk::PhysicalDevice&, vk::MemoryRequirements, vk::MemoryPropertyFlags);

    class Buffer {

        vk::Buffer handle;
        vk::DeviceMemory memory;
        void* data_location = nullptr;

        std::shared_ptr<Device> device;

        std::size_t size;

        bool device_local = false;
        bool persistent_memory = false;

        public:

        Buffer (std::shared_ptr<Device> device, std::size_t size, vk::BufferUsageFlags usage, bool device_local = false);
        ~Buffer ( );

        void write (auto data, std::size_t size = 0, bool persistent = false) {

            if (!size) size = get_size();

            if (device_local) {

                auto staging = Buffer(device, size, vk::BufferUsageFlagBits::eTransferSrc);

                staging.write(data, size);

                copy_buffer(device, staging.get_handle(), handle, size);

            } else {

                if (!data_location) data_location = device->get_handle().mapMemory(memory, 0, size);
                std::memcpy(data_location, data, size);

                if (!persistent) {
                    device->get_handle().unmapMemory(memory);
                    data_location = nullptr;
                } else persistent_memory = true;

            }

        };

        constexpr const vk::Buffer& get_handle ( ) const { return handle; };
        constexpr const std::size_t get_size ( ) const { return size; };

    };

}