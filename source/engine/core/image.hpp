#pragma once

#include <memory>

#include <vk_mem_alloc.h>

#include "device.hpp"

namespace engine {

    class Image {

        std::size_t width, height, size;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::UniqueImageView view;

        VmaAllocation allocation;
        vk::Sampler sampler;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::DescriptorSet descriptor_set;

        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        void create_handle ( );
        void create_sampler ( );
        void create_descriptor_set ( );

        public:

        Image (std::string_view path);
        Image (std::size_t width, std::size_t height, const std::vector<std::byte>& pixels);
        ~Image ( );

        void set_data(const std::vector<std::byte>& pixels);

        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }
        constexpr const vk::Sampler& get_sampler ( ) const { return sampler; }

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

        void create_handle ( );

        public:

        DepthImage (std::size_t width, std::size_t height);
        ~DepthImage ( );

        static vk::Format find_supported_format ();

        constexpr const vk::Image& get_handle ( ) const { return handle; }
        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }

    };

    vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags);

}