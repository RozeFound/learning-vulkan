#pragma once

#include <memory>
#include <span>

#include <vk_mem_alloc.h>

#include "device.hpp"

namespace engine {

    class Image {

        std::size_t width, height, size;
        uint32_t mip_levels;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::UniqueImageView view;

        VmaAllocation allocation;
        vk::UniqueSampler sampler;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::DescriptorSet descriptor_set;

        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        void create_handle ( );
        void create_sampler ( );
        void create_descriptor_set ( );
        void generate_mipmaps (const vk::CommandBuffer& command_buffer);

        public:

        Image (std::string_view path);
        Image (std::size_t width, std::size_t height, std::span<std::byte> pixels);
        ~Image ( );

        void set_data(std::span<std::byte> pixels);

        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }
        constexpr const vk::Sampler& get_sampler ( ) const { return sampler.get(); }

        constexpr const vk::DescriptorSet& get_descriptor_set ( ) const { return descriptor_set; }

        constexpr const std::size_t get_width ( ) const { return width; }
        constexpr const std::size_t get_height ( ) const { return height; }

    };

    class DepthImage {

        std::size_t width, height;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::UniqueImageView view;
        VmaAllocation allocation;

        vk::Format format = find_supported_format();

        public:

        DepthImage (std::size_t width, std::size_t height);
        ~DepthImage ( );

        static vk::Format find_supported_format ( );

        constexpr const vk::Image& get_handle ( ) const { return handle; }
        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }

    };

    vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags, uint32_t mip_levels = 1);

}