#pragma once

#include <memory>

#include "vulkan/vulkan.hpp"

#include "device.hpp"

namespace engine {

    vk::Pipeline create_pipeline (vk::PipelineLayout& layout);
    vk::PipelineLayout create_pipeline_layout (const vk::DescriptorSetLayout& layout);
    vk::DescriptorSetLayout create_descriptor_set_layout( );

}