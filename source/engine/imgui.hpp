#pragma once

#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

namespace engine {

    class ImGUI {

        vk::DescriptorPool descriptor_pool;

        Device device;
        SwapChain swapchain;
        PipeLine pipeline;

        uint32_t image_count;

        void init_imgui ( );
        void init_font ( );

        void make_descriptor_pool ( );

        public:

        ImGUI ( ) = default;
        ImGUI (Device&, SwapChain&, PipeLine&);

        void draw (vk::CommandBuffer&, std::function<void()>);

        void destroy ( );

    };

}