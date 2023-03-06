#pragma once

#include <functional>
#include <memory>

#include "core/device.hpp"
#include "core/swapchain.hpp"

#include "utils/mesh.hpp"

#include "ui_overlay.hpp"

namespace engine {

    class Engine {

        uint32_t max_frames_in_flight, frame_number = 0;
        const bool is_imgui_enabled = true;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        std::shared_ptr<Device> device;

        std::unique_ptr<UI> ui;
        std::unique_ptr<Mesh> asset;
        std::unique_ptr<Image> texture;

        std::unique_ptr<SwapChain> swapchain;

        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;

        vk::Queue graphics_queue;
        vk::Queue present_queue;

        vk::CommandPool command_pool;
        vk::DescriptorSetLayout descriptor_set_layout;

        void remake_swapchain ( );
        void make_command_pool ( );
        
        void prepare_frame (uint32_t index);
        void record_draw_commands (uint32_t index);

    public:

        bool is_framebuffer_resized = false;
        std::function<void()> on_ui_update = nullptr;

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw ();

    };

}