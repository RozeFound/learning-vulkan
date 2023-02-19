#pragma once

#include <vector>

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    class SwapChain {

        struct Frame {
            vk::Image image;
            vk::ImageView view;
            vk::Framebuffer buffer;
        };

        vk::SwapchainKHR handle;
        std::vector<Frame> frames;

        vk::SurfaceKHR surface;
        GLFWwindow* window;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        vk::SurfaceCapabilitiesKHR capabilities;
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;

        void query_swapchain_info ( );
        void make_frames ( );

        public:

        SwapChain ( ) = default;
        SwapChain (vk::PhysicalDevice&, vk::Device&, vk::SurfaceKHR&, GLFWwindow*);
        
        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle; };
        constexpr std::vector<Frame>& get_frames ( ) { return frames; };

        constexpr const vk::SurfaceFormatKHR& get_format ( ) const { return format; };
        constexpr const vk::Extent2D& get_extent ( ) const { return extent; };

        void destroy ( );

    };

}