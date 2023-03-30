#pragma once

#include <functional>
#include <memory>

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

#include "core/device.hpp"
#include "core/swapchain.hpp"
#include "core/model.hpp"

#include "ui_overlay.hpp"
#include "particle_system.hpp"

#include "utils/fps_limiter.hpp"
#include "utils/utils.hpp"

namespace engine {

    struct Object {

        Object (std::string_view texture_path, std::string_view model_path)
            : texture(texture_path), model(model_path) { };

        Object (std::string_view texture_path, std::span<Vertex> vertices, std::span<uint16_t> indices)
            : texture(texture_path), model(vertices, indices) { };

        Texture texture;
        Model model;

        constexpr void bind (const vk::CommandBuffer& commands, const vk::Pipeline& pipeline, const vk::PipelineLayout& layout) {

            auto offsets = std::array<vk::DeviceSize, 1> { }; 
            commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
            commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, 1, &texture.get_descriptor_set(), 0, nullptr);
            commands.bindVertexBuffers(0, 1, &model.get_vertex(), offsets.data());
            commands.bindIndexBuffer(model.get_index(), 0, vk::IndexType::eUint16);

        }

        constexpr void draw (const vk::CommandBuffer& commands) {

            commands.drawIndexed(model.get_indices_count(), 1, 0, 0, 0);

        }

    };

    struct Settings {

        bool vsync = false;
        bool gui_visible = false;
        int fps_limit = -1;

        GLZ_LOCAL_META(Settings, vsync, gui_visible, fps_limit);
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
        std::unique_ptr<ParticleSystem> particle_system;
        std::unique_ptr<SwapChain> swapchain;

        vk::Pipeline pipeline;
        vk::RenderPass render_pass;
        vk::PipelineLayout pipeline_layout;
        vk::DescriptorSetLayout descriptor_set_layout;

        vk::Queue queue;
        vk::CommandPool command_pool;

        void setup_particles ( );

        void make_command_pool ( );
        void make_command_buffers ( );
        
        void apply_camera_transformation (uint32_t index);
        void record_draw_commands (uint32_t index, std::function<void()> draw_callback);
        void submit (uint32_t index);

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
            }

            is_settings_changed = true;

        }

        constexpr Settings get_settings ( ) const { return settings; }
        void apply_settings ( );

        Engine(GLFWwindow* window);
        ~Engine ( );

        void draw (std::shared_ptr<Object> object);

    };

}