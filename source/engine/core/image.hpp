#pragma once

#include <memory>
#include <span>

#include <vk_mem_alloc.h>

#include "device.hpp"

namespace engine {

    class Image {

        protected:

        std::size_t width, height;
        uint32_t mip_levels;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::UniqueImageView view;
        VmaAllocation allocation;

        vk::Format format;
        vk::ImageUsageFlags usage;
        vk::SampleCountFlagBits sample_count;

        virtual void create_handle ( );

        public:

        Image ( ) = default;
        Image (std::size_t width, std::size_t height, vk::Format format, vk::ImageUsageFlags usage, 
        uint32_t mip_levels, vk::SampleCountFlagBits sample_count) :
            width(width), height(height), format(format), usage(usage), 
            mip_levels(mip_levels), sample_count(sample_count) { create_handle(); }
        ~Image ( ) { vmaDestroyImage(device->get_allocator(), VkImage(handle), allocation); };

        static vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags, uint32_t mip_levels = 1);

        static vk::Format get_depth_format ( );

        constexpr const vk::Image& get_handle ( ) const { return handle; }
        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }

        constexpr const std::size_t get_width ( ) const { return width; }
        constexpr const std::size_t get_height ( ) const { return height; }

    };

    class Texture : public Image {

        std::size_t size;

        vk::UniqueSampler sampler;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::DescriptorSet descriptor_set;

        void create_handle ( ) override;
        void create_sampler ( );
        void create_descriptor_set ( );
        void generate_mipmaps (const vk::CommandBuffer& command_buffer);

        public:

        Texture (std::string_view path);
        Texture (std::size_t width, std::size_t height, std::span<std::byte> pixels);

        void set_data(std::span<std::byte> pixels);

        constexpr const vk::Sampler& get_sampler ( ) const { return sampler.get(); }
        constexpr const vk::DescriptorSet& get_descriptor_set ( ) const { return descriptor_set; }

    };

}