#pragma once

#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "pipeline.hpp"
#include "swapchain.hpp"

namespace engine {

    class ImGUI {

        vk::CommandPool command_pool; 
        vk::DescriptorPool descriptor_pool;

        std::vector<vk::CommandBuffer> command_buffers;
        std::vector<vk::Framebuffer> frame_buffers;

        GLFWwindow* window;
        vk::Instance instance;
        vk::SurfaceKHR surface;

        SwapChain swapchain;
        PipeLine pipeline;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        uint32_t image_count;

        void init_imgui ( );
        void init_font ( );

        void make_command_pool ( );
        void make_descriptor_pool ( );

        void make_framebuffers ( );
        void make_command_buffers ( );

        public:

        ImGUI ( ) = default;
        ImGUI (vk::PhysicalDevice&, vk::Device&, vk::Instance&, vk::SurfaceKHR&, SwapChain&, PipeLine&, GLFWwindow*);

        vk::CommandBuffer& get_commands (uint32_t index, std::function<void()>);

        void destroy ( );

    };

}