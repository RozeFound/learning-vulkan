#pragma once

#include <memory>
#include <span>

#include <vk_mem_alloc.h>

#include "device.hpp"

namespace engine {

    class Image {

        protected:

        std::size_t width, height;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::UniqueImageView view;
        VmaAllocation allocation;

        public:

        Image ( ) = default;
        Image (std::size_t width, std::size_t height) : width(width), height(height) { }
        ~Image ( ) { vmaDestroyImage(device->get_allocator(), VkImage(handle), allocation); };

        constexpr const vk::Image& get_handle ( ) const { return handle; }
        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }

        constexpr const std::size_t get_width ( ) const { return width; }
        constexpr const std::size_t get_height ( ) const { return height; }

    };

    class TexImage : public Image {

        std::size_t size;
        uint32_t mip_levels;

        vk::UniqueSampler sampler;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::DescriptorSet descriptor_set;

        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        void create_handle ( );
        void create_sampler ( );
        void create_descriptor_set ( );
        void generate_mipmaps (const vk::CommandBuffer& command_buffer);

        public:

        TexImage (std::string_view path);
        TexImage (std::size_t width, std::size_t height, std::span<std::byte> pixels);

        void set_data(std::span<std::byte> pixels);

        constexpr const vk::Sampler& get_sampler ( ) const { return sampler.get(); }
        constexpr const vk::DescriptorSet& get_descriptor_set ( ) const { return descriptor_set; }

    };

    class ColorImage : public Image {

        public:

        ColorImage (std::size_t width, std::size_t height);

    };

    class DepthImage : public Image {

        vk::Format format = find_supported_format ( );

        public:

        DepthImage (std::size_t width, std::size_t height);

        static vk::Format find_supported_format ( );

    };

    vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags, uint32_t mip_levels = 1);

}