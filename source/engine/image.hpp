#pragma once

#include <memory>

#include "device.hpp"

namespace engine {

    class Image {

        std::size_t width, height, size;

        std::shared_ptr<Device> device;

        vk::Image handle;
        vk::ImageView view;
        vk::DeviceMemory memory;
        vk::Sampler sampler;

        vk::Format format = vk::Format::eR8G8B8A8Srgb;

        void create_handle ( );
        void create_view ( );
        void create_sampler ( );

        public:

        Image (std::shared_ptr<Device> device, std::string_view path);
        Image (std::shared_ptr<Device> device, std::size_t width, std::size_t height, std::vector<std::byte> pixels);
        ~Image ( );

        void set_data(std::vector<std::byte> pixels);

        constexpr const vk::ImageView& get_view ( ) const { return view; };
        constexpr const vk::Sampler& get_sampler ( ) const { return sampler; };

        constexpr const std::size_t get_width ( ) const { return width; };
        constexpr const std::size_t get_height ( ) const { return height; };



    };

}