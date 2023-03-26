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

        constexpr void set_gui_visible (bool state) { settings.gui_visible = state; is_settings_changed = true; }
        constexpr void set_vsync (bool state) { settings.vsync = state; is_settings_changed = true; }
        constexpr void set_fps_limit (int state) { settings.fps_limit = state; is_settings_changed = true; }

        constexpr Settings get_settings ( ) { return settings; }
        void apply_settings ( );

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (std::shared_ptr<Object> object);

    };

}