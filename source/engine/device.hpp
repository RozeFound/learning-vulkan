#pragma once

#include <limits>
#include <memory>

#include "essentials.hpp"
#include "utils.hpp"

namespace engine {

    class Device {

        vk::Device handle;
        vk::PhysicalDevice gpu;
        vk::SurfaceKHR surface;
        vk::Instance instance;
        GLFWwindow* window;

        void create_handle ( );
        void choose_physical_device ( );

        void make_instance ( );
        void make_surface ( );

        public:

        Device (GLFWwindow*);
        ~Device ( );

        static void set_static_instance (std::shared_ptr<Device>&);
        const static std::shared_ptr<Device> get ( );

        constexpr const vk::Device& get_handle ( ) const { return handle; }
        constexpr const vk::PhysicalDevice& get_gpu ( ) const { return gpu; }
        constexpr const vk::SurfaceKHR& get_surface ( ) const { return surface; }
        constexpr const vk::Instance& get_instance ( ) const { return instance; }
        constexpr const GLFWwindow* get_window ( ) const { return window; }

        constexpr const vk::Extent2D get_extent ( ) const {

            auto capabilities = gpu.getSurfaceCapabilitiesKHR(surface);

            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
                return capabilities.currentExtent;
                
            int width, height;
            glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &width, &height);

            auto min_width = std::max(capabilities.minImageExtent.width, to_u32(width));
            auto min_height = std::max(capabilities.minImageExtent.height, to_u32(height));

            return vk::Extent2D { 
                .width = std::min(min_width, capabilities.maxImageExtent.width),
                .height = std::min(min_height, capabilities.maxImageExtent.height)
            };

        } 

        constexpr const vk::SurfaceFormatKHR get_format ( ) const {

            auto formats =  gpu.getSurfaceFormatsKHR(surface);

            for (const auto& format : formats)
                if (format.format == vk::Format::eB8G8R8A8Unorm 
                    && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                    return format;

            return formats.at(0);

        }

    };

}