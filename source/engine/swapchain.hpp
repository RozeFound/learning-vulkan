#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "memory.hpp"

namespace engine {

    struct UniformBufferObject {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 projection;
    };

    class SwapChain {

        struct Frame {
            vk::Image image;
            vk::ImageView view;
            vk::Framebuffer buffer;
            vk::CommandBuffer commands;
            vk::DescriptorSet descriptor_set;

            Buffer uniform_buffer;

            vk::Semaphore image_available;
            vk::Semaphore render_finished;
            vk::Fence in_flight;
        };

        vk::SwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass renderpass;

        vk::SurfaceCapabilitiesKHR capabilities;
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;

        Device device;

        void make_frames ( );
        void make_framebuffers ( );

        public:

        SwapChain ( ) = default;
        SwapChain (Device& device, const vk::RenderPass&);

        void create_handle ( );

        void make_commandbuffers (vk::CommandPool&);
        void make_descriptor_sets (vk::DescriptorPool&, const vk::DescriptorSetLayout&);

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