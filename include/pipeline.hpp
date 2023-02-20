#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    class PipeLine {

        vk::Pipeline handle;
        vk::PipelineLayout layout;
        vk::RenderPass renderpass;

        vk::Device device;
        vk::SurfaceFormatKHR format;

        void create_layout ( );
        void create_renderpass ( );

        public:

        PipeLine ( ) = default;
        PipeLine (vk::Device& device, const vk::SurfaceFormatKHR& format);

        constexpr const vk::Pipeline& get_handle ( ) const { return handle; };
        constexpr const vk::PipelineLayout& get_layout ( ) const { return layout; };
        constexpr vk::RenderPass& get_renderpass ( ) { return renderpass; };

        void destroy ( );

    };

}