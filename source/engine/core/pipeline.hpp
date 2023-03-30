#pragma once

#include <memory>
#include <vector>

#include "device.hpp"
#include "shaders.hpp"

#include "../utils/primitives.hpp"

namespace engine {

    vk::PipelineLayout create_pipeline_layout (const vk::DescriptorSetLayout* layout = nullptr, vk::PushConstantRange* range = nullptr);

    vk::PipelineInputAssemblyStateCreateInfo create_input_assembly_info (vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList);
    vk::PipelineRasterizationStateCreateInfo create_rasterization_info (vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack);

    vk::PipelineMultisampleStateCreateInfo create_multisampling_info (
        vk::SampleCountFlagBits sample_count = vk::SampleCountFlagBits::e1,
        bool sample_shading = false
    );

    vk::PipelineDepthStencilStateCreateInfo create_depth_stencil_info 
        (bool depth_test = true, bool depth_write = true, bool stencil_test = false);

    vk::PipelineColorBlendAttachmentState create_color_blend_attachment (bool enable_blend = false,
        std::array<vk::BlendFactor, 2> color_blend_factor = { }, vk::BlendOp color_blend_op = { },
        std::array<vk::BlendFactor, 2> alpha_blend_factor = { }, vk::BlendOp alpha_blend_op = { }
    );

    struct PipeLineCreateInfo {

        const vk::VertexInputBindingDescription binding_description = Vertex::get_binding_description();
        const std::vector<vk::VertexInputAttributeDescription> attribute_descriptions = Vertex::get_attribute_descriptions();

        const vk::PipelineInputAssemblyStateCreateInfo input_assembly_info = create_input_assembly_info();
        const vk::PipelineRasterizationStateCreateInfo rasterization_info = create_rasterization_info();
        const vk::PipelineMultisampleStateCreateInfo multisampling_info = create_multisampling_info();
        const vk::PipelineDepthStencilStateCreateInfo depth_stencil_info = create_depth_stencil_info();
        const vk::PipelineColorBlendAttachmentState color_blend_attachment = create_color_blend_attachment();

        const vk::PipelineLayout& layout; 
        const vk::RenderPass& render_pass = nullptr;
        const std::string shader_path;

    };

    vk::RenderPass create_render_pass ( );
    vk::Pipeline create_pipeline (const PipeLineCreateInfo& create_info);
    vk::Pipeline create_compute_pipeline (const vk::PipelineLayout& layout, std::string shader_path);
    vk::DescriptorSetLayout create_descriptor_set_layout ( );

}