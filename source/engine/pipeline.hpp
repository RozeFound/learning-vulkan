#pragma once

#include <memory>

#include "device.hpp"
#include "essentials.hpp"

namespace engine {

    vk::Pipeline create_pipeline (vk::PipelineLayout& layout, vk::RenderPass& renderpass);
    vk::PipelineLayout create_pipeline_layout (const vk::DescriptorSetLayout& layout);
    vk::DescriptorSetLayout create_descriptor_set_layout( );
    vk::RenderPass create_renderpass ( );

}