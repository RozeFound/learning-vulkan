#pragma once

#include <memory>

#include "device.hpp"
#include "essentials.hpp"

namespace engine {

    vk::Pipeline create_pipeline (std::shared_ptr<Device> device, vk::PipelineLayout& layout, vk::RenderPass& renderpass);
    vk::PipelineLayout create_pipeline_layout (std::shared_ptr<Device> device, const vk::DescriptorSetLayout& layout);
    vk::DescriptorSetLayout create_descriptor_set_layout(std::shared_ptr<Device> device);
    vk::RenderPass create_renderpass (std::shared_ptr<Device> device);

}