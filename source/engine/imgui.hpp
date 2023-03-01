#pragma once

#include <functional>
#include <vector>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include "essentials.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

namespace engine {

    class ImGUI {

        vk::DescriptorPool descriptor_pool;

        std::shared_ptr<Device> device = Device::get();
        vk::RenderPass renderpass;

        uint32_t image_count;

        void init_imgui ( );
        void init_font ( );

        void make_descriptor_pool ( );

        public:

        ImGUI (std::size_t image_count, vk::RenderPass& renderpass);
        ~ImGUI ( );

        void draw (vk::CommandBuffer&, std::function<void()>);

    };

}