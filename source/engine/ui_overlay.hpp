#pragma once

#include <memory>

#include <backends/imgui_impl_glfw.h>

#include "core/device.hpp"
#include "core/memory.hpp"
#include "core/image.hpp"

namespace engine {

    class UI {

        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
        vk::DescriptorSetLayout descriptor_set_layout;

        std::shared_ptr<Device> device = Device::get();

        std::unique_ptr<Image> font_texture;
        std::vector<std::unique_ptr<Buffer>> vertex_buffers;
        std::vector<std::unique_ptr<Buffer>> index_buffers;

        uint32_t image_count;

        void create_handle ( );
        void create_font_texture ( );

        bool update_buffers (uint32_t index);

        public:

        UI (uint32_t image_count) : image_count(image_count) { create_handle(); }
        ~UI ( );

        static void new_frame();
        void draw (vk::CommandBuffer&, uint32_t index);
        static void end_frame();

    };

}