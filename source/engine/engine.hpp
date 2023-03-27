#pragma once

#include <functional>
#include <memory>

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include "core/device.hpp"
#include "core/swapchain.hpp"
#include "core/model.hpp"

#include "ui_overlay.hpp"

#include "utils/fps_limiter.hpp"
#include "utils/utils.hpp"

namespace engine {

    struct Object {
        Texture texture;
        Model model;
    };

    struct Settings {

        bool vsync = false;
        int fps_limit = -1;
        bool gui_visible = false;

        GLZ_LOCAL_META(Settings, vsync, fps_limit, gui_visible);
    };

    class Engine {

        uint32_t max_frames_in_flight, current_frame = 0;
        bool is_imgui_enabled = true;

        Settings settings;
        bool is_settings_changed = true;
        FPSLimiter fps_limiter;

        vk::DebugUtilsMessengerEXT debug_messenger;
        vk::DispatchLoaderDynamic dldi;

        std::shared_ptr<Device> device;

        std::unique_ptr<UI> ui;
        std::unique_ptr<SwapChain> swapchain;

        vk::Pipeline pipeline;
        vk::RenderPass render_pass;
        vk::PipelineLayout pipeline_layout;
        vk::DescriptorSetLayout descriptor_set_layout;

        vk::Queue queue;
        vk::CommandPool command_pool;

        void remake_swapchain ( );
        void make_command_pool ( );
        void make_command_buffers ( );
        
        void prepare_frame (uint32_t index);
        void record_draw_commands (uint32_t index, std::shared_ptr<Object> object);

    public:

        bool is_framebuffer_resized = false;
        std::function<void()> ui_callback = nullptr;

        constexpr void ui_key_callback(auto&&...args) {
            if (is_imgui_enabled) ImGui_ImplGlfw_KeyCallback(args...);
        }

        template <fixed_string key> constexpr void set (const auto value) {
            if constexpr (key == fixed_string("vsync")) settings.vsync = value;
            if constexpr (key == fixed_string("fps_limit")) settings.fps_limit = value;
            if constexpr (key == fixed_string("gui_visible")) { 
                if (is_imgui_enabled) settings.gui_visible = value;
                return;
            } is_settings_changed = true;
        }

        constexpr Settings get_settings ( ) { return settings; }
        void apply_settings ( );

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (std::shared_ptr<Object> object);

    };

}