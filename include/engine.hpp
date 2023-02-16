#pragma once

#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    class Engine {

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        std::size_t width = 800, height = 600;
        std::string_view title = "Learning Vulkan";

        GLFWwindow* window;
        vk::Instance instance;
        vk::SurfaceKHR surface;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        vk::SwapchainKHR swapchain;
        vk::Queue graphics_queue;
        vk::Queue present_queue;

        void make_window ( );
        void make_instance ( );
        void make_device ( );
        void make_swapchain ( );

    public:

        Engine();
        ~Engine();

        auto get_dimensions ( ) const { return std::pair(width, height); };
        auto get_instance ( ) const { return instance; };
        auto get_devices ( ) const { return std::pair(physical_device, device); } ;
        auto get_surface ( ) const { return surface; };

    };

}