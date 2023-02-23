#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include "device.hpp"
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
        const bool is_imgui_enabled = true;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        Device device;

        ImGUI imgui;
        Mesh* asset;

        SwapChain swapchain;
        PipeLine pipeline;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        vk::CommandPool command_pool;
        vk::DescriptorPool descriptor_pool;

        void prepare ( );
        void prepare_frame (uint32_t index);

        void remake_swapchain ( );

        void make_command_pool ( );
        void make_descriptor_pool ( );
        
        void record_draw_commands (uint32_t index, Scene& scene);

    public:

        bool is_framebuffer_resized = false;
        std::function<void()> on_render = nullptr;

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (Scene& scene);

    };

}