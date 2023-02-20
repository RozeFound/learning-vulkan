#pragma once

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "swapchain.hpp"
#include "pipeline.hpp"
#include "scene.hpp"

namespace engine {

    class Engine {

        int max_frames_in_flight, frame_number;

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
        void prepare ( );

        void remake_swapchain ( );

        void make_command_pool ( );
        void record_draw_commands (uint32_t index, Scene& scene);

    public:

        bool is_framebuffer_resized = false;

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (Scene& scene);

        constexpr const auto& get_instance ( ) const { return instance; };
        constexpr const auto get_devices ( ) const { return std::pair(physical_device, device); } ;
        constexpr const auto& get_surface ( ) const { return surface; };

    };

}