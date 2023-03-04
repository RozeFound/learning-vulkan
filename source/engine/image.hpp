#pragma once

#include <memory>

#include "device.hpp"

namespace engine {

    class Image {

        std::size_t width, height, size;

        std::shared_ptr<Device> device = Device::get();

        vk::Image handle;
        vk::ImageView view;

        vk::DeviceMemory memory;
        vk::Sampler sampler;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::DescriptorSet descriptor_set;

        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        void create_handle ( );
        void create_view ( );
        void create_sampler ( );
        void create_descriptor_set ( );

        public:

        Image (std::string_view path);
        Image (std::size_t width, std::size_t height, std::vector<std::byte> pixels);
        ~Image ( );

        void set_data(std::vector<std::byte> pixels);

        constexpr const vk::ImageView& get_view ( ) const { return view; }
        constexpr const vk::Sampler& get_sampler ( ) const { return sampler; }

        constexpr const vk::DescriptorSet& get_descriptor_set ( ) const { return descriptor_set; }

        constexpr const std::size_t get_width ( ) const { return width; }
        constexpr const std::size_t get_height ( ) const { return height; }

    };

    class DepthImage {

        std::size_t width, height;

        std::shared_ptr<Device> device = Device::get();

        vk::UniqueImage handle;
        vk::UniqueImageView view;
        vk::UniqueDeviceMemory memory;

        void create_handle ( );
        void create_view ( );

        public:

        DepthImage (std::size_t width, std::size_t height);

        static vk::Format find_supported_format ();

        constexpr const vk::Image& get_handle ( ) const { return handle.get(); }
        constexpr const vk::ImageView& get_view ( ) const { return view.get(); }

    };

}