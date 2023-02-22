#pragma once


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

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

        Device ( ) = default;
        Device (GLFWwindow*);
        void destroy ( );

        constexpr const vk::Device& get_handle ( ) const { return handle; };
        constexpr const vk::PhysicalDevice& get_gpu ( ) const { return gpu; };
        constexpr const vk::SurfaceKHR& get_surface ( ) const { return surface; };
        constexpr const vk::Instance& get_instance ( ) const { return instance; };
        constexpr const GLFWwindow* get_window ( ) const { return window; };

    };

}