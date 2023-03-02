#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace engine {

    void copy_buffer (const vk::Buffer& source, vk::Buffer& destination, std::size_t size);
    uint32_t get_memory_index (vk::MemoryRequirements, vk::MemoryPropertyFlags);

    void insert_image_memory_barrier (const vk::CommandBuffer& command_buffer, const vk::Image& image,
        vk::ImageAspectFlags aspect_flags, const std::array<vk::PipelineStageFlags, 2> stages, 
        const std::array<vk::AccessFlags, 2> access_masks, const std::array<vk::ImageLayout, 2> layouts) ;


    class Buffer {

        vk::UniqueBuffer handle;
        vk::UniqueDeviceMemory memory;
        void* data_location = nullptr;

        std::shared_ptr<Device> device = Device::get();

        std::size_t size;

        bool device_local = false;
        bool persistent_memory = false;

        public:

        Buffer (std::size_t size, vk::BufferUsageFlags usage, bool device_local = false);
        ~Buffer ( );

        void write (auto data, std::size_t size = 0, bool persistent = false) {

            if (!size) size = get_size();

            if (device_local) {

                auto staging = Buffer(size, vk::BufferUsageFlagBits::eTransferSrc);

                staging.write(data, size);

                copy_buffer(staging.get_handle(), handle.get(), size);

            } else {

                if (!data_location) data_location = device->get_handle().mapMemory(memory.get(), 0, size);
                std::memcpy(data_location, data, size);

                if (!persistent) {
                    device->get_handle().unmapMemory(memory.get());
                    data_location = nullptr;
                } else persistent_memory = true;

            }

        };

        constexpr const vk::Buffer& get_handle ( ) const { return handle.get(); }
        constexpr const std::size_t get_size ( ) const { return size; }

    };

}