#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include "mesh.hpp"

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "swapchain.hpp"
#include "pipeline.hpp"
#include "../scene.hpp"
#include "imgui.hpp"

namespace engine {

    class Engine {

        uint32_t max_frames_in_flight, frame_number;
        const bool is_imgui_enabled = false;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        GLFWwindow* window;
        vk::Instance instance;
        vk::SurfaceKHR surface;

        vk::PhysicalDevice physical_device;
        vk::Device device;

        ImGUI imgui;
        Mesh* asset;

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
        std::function<void()> on_render = nullptr;

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (Scene& scene);

        constexpr const auto& get_instance ( ) const { return instance; };
        constexpr const auto get_devices ( ) const { return std::pair(physical_device, device); } ;
        constexpr const auto& get_surface ( ) const { return surface; };

    };

}