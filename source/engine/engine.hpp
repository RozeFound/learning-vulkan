#pragma once

#include <functional>

#include <vulkan/vulkan.hpp>

#include "mesh.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "imgui.hpp"

namespace engine {

    class Engine {

        uint32_t max_frames_in_flight, frame_number = 0;
        const bool is_imgui_enabled = false;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        std::shared_ptr<Device> device;

        std::unique_ptr<ImGUI> imgui;
        std::unique_ptr<Mesh> asset;

        std::unique_ptr<SwapChain> swapchain;

        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        vk::CommandPool command_pool;
        vk::DescriptorPool descriptor_pool;
        vk::DescriptorSetLayout descriptor_set_layout;

        void remake_swapchain ( );
        void make_command_pool ( );
        void make_descriptor_pool ( );
        
        void prepare_frame (uint32_t index);
        void record_draw_commands (uint32_t index);

    public:

        bool is_framebuffer_resized = false;
        std::function<void()> on_render = nullptr;

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw ();

    };

}