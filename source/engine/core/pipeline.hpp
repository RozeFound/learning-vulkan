#pragma once

#include <memory>

#include "device.hpp"
#include "shaders.hpp"

namespace engine {

    vk::Pipeline create_pipeline (vk::PipelineLayout& layout);
    vk::Pipeline create_ui_pipeline (vk::PipelineLayout& layout);
    vk::PipelineLayout create_pipeline_layout (const vk::DescriptorSetLayout* layout, vk::PushConstantRange* range);
    vk::DescriptorSetLayout create_descriptor_set_layout( );

}