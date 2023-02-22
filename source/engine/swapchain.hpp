#pragma once

#include <vector>

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace engine {

    class SwapChain {

        struct Frame {
            vk::Image image;
            vk::ImageView view;
            vk::Framebuffer buffer;
            vk::CommandBuffer commands;

            vk::Semaphore image_available;
            vk::Semaphore render_finished;
            vk::Fence in_flight;
        };

        vk::SwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass renderpass;
        vk::CommandPool command_pool;

        vk::SurfaceCapabilitiesKHR capabilities;
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;

        Device device;

        void make_framebuffers ( );
        void make_commandbuffers ( );
        void make_frames ( );

        public:

        SwapChain ( ) = default;
        SwapChain (Device& device, const vk::RenderPass&, vk::CommandPool&);

        void create_handle ( );

        static vk::Extent2D query_extent (const vk::PhysicalDevice&, const vk::SurfaceKHR&, const GLFWwindow*);
        static vk::Extent2D query_extent (vk::SurfaceCapabilitiesKHR&, const GLFWwindow*);
        
        static vk::SurfaceFormatKHR query_format (const vk::PhysicalDevice&, const vk::SurfaceKHR&);
        
        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle; };
        constexpr std::vector<Frame>& get_frames ( ) { return frames; };

        constexpr const vk::SurfaceFormatKHR& get_format ( ) const { return format; };
        constexpr const vk::Extent2D& get_extent ( ) const { return extent; };

        void destroy ( );

    };

}