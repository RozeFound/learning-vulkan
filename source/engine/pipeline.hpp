#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    class PipeLine {

        vk::Pipeline handle;

        vk::PipelineLayout pipeline_layout;
        vk::DescriptorSetLayout descriptor_set_layout;

        vk::RenderPass renderpass;

        vk::Device device;
        vk::SurfaceFormatKHR format;

        void create_layout ( );
        void create_renderpass ( );

        public:

        PipeLine ( ) = default;
        PipeLine (const vk::Device& device, const vk::SurfaceFormatKHR& format);

        constexpr const vk::Pipeline& get_handle ( ) const { return handle; };
        constexpr const vk::PipelineLayout& get_pipeline_layout ( ) const { return pipeline_layout; };
        constexpr const vk::DescriptorSetLayout& get_descriptor_set_layout ( ) const { return descriptor_set_layout; };
        constexpr const vk::RenderPass& get_renderpass ( ) const { return renderpass; };

        void destroy ( );

    };

}