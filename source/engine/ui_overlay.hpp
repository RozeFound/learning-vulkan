#pragma once

#include <memory>
#include <source_location>

#include <backends/imgui_impl_glfw.h>

#include "core/device.hpp"
#include "core/memory.hpp"
#include "core/image.hpp"

namespace engine {

    class UI {

        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
        vk::DescriptorSetLayout descriptor_set_layout;
        vk::RenderPass render_pass;

        std::shared_ptr<Device> device = Device::get();

        std::unique_ptr<Texture> font_texture;
        std::vector<std::unique_ptr<Buffer>> vertex_buffers;
        std::vector<std::unique_ptr<Buffer>> index_buffers;

        uint32_t image_count;

        void create_handle ( );
        void create_font_texture ( );

        bool update_buffers (uint32_t index);

        public:

        UI (uint32_t image_count, vk::RenderPass render_pass) 
            : image_count(image_count), render_pass(render_pass) { create_handle(); }
        ~UI ( );

        static ScopedTimer add_perf_counter (std::source_location = std::source_location::current());

        static void new_frame();
        void draw (const vk::CommandBuffer&, uint32_t index);
        static void end_frame();

    };

}

#define SCOPED_PERF_LOG auto timer__LINE__ = engine::UI::add_perf_counter()