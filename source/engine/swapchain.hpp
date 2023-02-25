#pragma once

#include <vector>

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "memory.hpp"

namespace engine {

    class SwapChain {

        struct Frame {
            vk::Image image;
            vk::ImageView view;
            vk::Framebuffer buffer;
            vk::CommandBuffer commands;
            vk::DescriptorSet descriptor_set;

            Buffer uniform_buffer;
            Buffer storage_buffer;

            vk::Semaphore image_available;
            vk::Semaphore render_finished;
            vk::Fence in_flight;
        };

        vk::SwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass renderpass;
        vk::Extent2D extent;

        Device device;

        void make_frames ( );
        void make_framebuffers ( );

        public:

        SwapChain ( ) = default;
        SwapChain (Device& device, const vk::RenderPass& renderpass)
            : device(device), renderpass(renderpass) { create_handle(); };

        void create_handle ( );

        void make_commandbuffers (vk::CommandPool&);
        void make_descriptor_sets (vk::DescriptorPool&, const vk::DescriptorSetLayout&);
        
        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle; };
        constexpr std::vector<Frame>& get_frames ( ) { return frames; };
        constexpr const vk::Extent2D& get_extent( ) const { return extent; };

        void destroy ( );

    };

}