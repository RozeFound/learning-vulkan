#pragma once

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "swapchain.hpp"
#include "pipeline.hpp"

namespace engine {

    class Engine {

        const int max_frames_in_flight = 2;
        uint32_t frame_number = 0;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        GLFWwindow* window;
        vk::Instance instance;
        vk::SurfaceKHR surface;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        SwapChain swapchain;
        PipeLine pipeline;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        vk::CommandPool command_pool;

        void make_window ( );
        void make_instance ( );
        void make_device ( );

        void make_framebuffers ( );
        void make_command_pool ( );
        void make_commandbuffers ( );
        void record_draw_commands (uint32_t index);

    public:

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw ( );

        constexpr const auto& get_instance ( ) const { return instance; };
        constexpr const auto get_devices ( ) const { return std::pair(physical_device, device); } ;
        constexpr const auto& get_surface ( ) const { return surface; };

    };

}