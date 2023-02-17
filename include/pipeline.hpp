#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "swapchain.hpp"

namespace engine {

    class PipeLine {

        vk::Pipeline handle;
        vk::PipelineLayout layout;
        vk::RenderPass renderpass;

        vk::Device device;
        SwapChain swapchain;

        void create_layout ( );
        void create_renderpass ( );

        public:

        PipeLine ( ) = default;
        PipeLine (vk::Device& device, SwapChain& swapchain);

        constexpr vk::Pipeline get_handle ( ) const { return handle; };

        void destroy ( );

    };

}