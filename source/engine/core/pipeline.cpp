#include "pipeline.hpp"

#include "device.hpp"
#include "image.hpp"

#include "../utils/logging.hpp"
#include "../utils/primitives.hpp"
#include "../utils/utils.hpp"

namespace engine {

    vk::Pipeline create_pipeline (vk::PipelineLayout& layout) {

        auto device = Device::get();

        auto binding_description = Vertex::get_binding_description();
        auto attribute_descriptions = Vertex::get_attribute_descriptions(); 

        auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {
            .flags = vk::PipelineVertexInputStateCreateFlags(),
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding_description,
            .vertexAttributeDescriptionCount = to_u32(attribute_descriptions.size()),
            .pVertexAttributeDescriptions = attribute_descriptions.data()
        };

        auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo {
            .flags = vk::PipelineInputAssemblyStateCreateFlags(),
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = VK_FALSE
        };

        auto dynamic_states = std::vector { 
            vk::DynamicState::eViewport, 
            vk::DynamicState::eScissor
        };

        auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo {
            .flags = vk::PipelineDynamicStateCreateFlags(),
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        auto viewport_state_info = vk::PipelineViewportStateCreateInfo {
            .viewportCount = 1,
            .scissorCount = 1,
        };

        auto rasterizer = vk::PipelineRasterizationStateCreateInfo {
            .flags = vk::PipelineRasterizationStateCreateFlags(),
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f
        };

        auto multisampling = vk::PipelineMultisampleStateCreateInfo {
            .flags = vk::PipelineMultisampleStateCreateFlags(),
            .rasterizationSamples = get_max_sample_count(device->get_gpu()),
            .sampleShadingEnable = VK_TRUE
        };

        auto depth_stencil = vk::PipelineDepthStencilStateCreateInfo {
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = vk::CompareOp::eLess,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE
        };

        auto color_blend_attachment = vk::PipelineColorBlendAttachmentState {
            .blendEnable = VK_FALSE,
            .colorWriteMask = vk::ColorComponentFlagBits::eR 
                            | vk::ColorComponentFlagBits::eG 
                            | vk::ColorComponentFlagBits::eB 
                            | vk::ColorComponentFlagBits::eA
        };

        auto color_blend_info = vk::PipelineColorBlendStateCreateInfo {
            .flags = vk::PipelineColorBlendStateCreateFlags(),
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment
        };

        auto color_attachment_format = device->get_format().format;
        auto depth_attachment_format = DepthImage::find_supported_format();

        auto rendering_info = vk::PipelineRenderingCreateInfoKHR {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format,
            .depthAttachmentFormat = depth_attachment_format
        };

        auto shader = Shader("shaders/basic");
        auto stages = shader.get_stage_info(); 

        auto create_info = vk::GraphicsPipelineCreateInfo {
            .pNext = &rendering_info,
            .flags = vk::PipelineCreateFlags(),
            .stageCount = to_u32(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_info,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        try {
            auto result = device->get_handle().createGraphicsPipeline(nullptr, create_info);
            logi("Successfully created Graphics PipeLine");
            return result.value;
        } catch (vk::SystemError err) {
            loge("Failed to create Graphics Pipeline");
            return nullptr;
        }
    
    }

    vk::Pipeline create_ui_pipeline (vk::PipelineLayout& layout) {

        auto device = Device::get();

        auto binding_description = ImVertex::get_binding_description();
        auto attribute_descriptions = ImVertex::get_attribute_descriptions(); 

        auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {
            .flags = vk::PipelineVertexInputStateCreateFlags(),
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding_description,
            .vertexAttributeDescriptionCount = to_u32(attribute_descriptions.size()),
            .pVertexAttributeDescriptions = attribute_descriptions.data()
        };

        auto input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo {
            .flags = vk::PipelineInputAssemblyStateCreateFlags(),
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = VK_FALSE
        };

        auto dynamic_states = std::vector { 
            vk::DynamicState::eViewport, 
            vk::DynamicState::eScissor
        };

        auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo {
            .flags = vk::PipelineDynamicStateCreateFlags(),
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        auto viewport_state_info = vk::PipelineViewportStateCreateInfo {
            .viewportCount = 1,
            .scissorCount = 1,
        };

        auto rasterizer = vk::PipelineRasterizationStateCreateInfo {
            .flags = vk::PipelineRasterizationStateCreateFlags(),
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eNone,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f
        };

        // Multisampled text is a bit too much btw
        auto multisampling = vk::PipelineMultisampleStateCreateInfo {
            .flags = vk::PipelineMultisampleStateCreateFlags(),
            .rasterizationSamples = get_max_sample_count(device->get_gpu()),
            .sampleShadingEnable = VK_FALSE
        };

        auto depth_stencil = vk::PipelineDepthStencilStateCreateInfo {
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE
        };

        auto color_blend_attachment = vk::PipelineColorBlendAttachmentState {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR 
                            | vk::ColorComponentFlagBits::eG 
                            | vk::ColorComponentFlagBits::eB 
                            | vk::ColorComponentFlagBits::eA
        };

        auto color_blend_info = vk::PipelineColorBlendStateCreateInfo {
            .flags = vk::PipelineColorBlendStateCreateFlags(),
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment
        };

        auto color_attachment_format = device->get_format().format;
        auto depth_attachment_format = DepthImage::find_supported_format();

        auto rendering_info = vk::PipelineRenderingCreateInfoKHR {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format,
            .depthAttachmentFormat = depth_attachment_format
        };

        auto shader = Shader("shaders/imgui");
        auto stages = shader.get_stage_info(); 

        auto create_info = vk::GraphicsPipelineCreateInfo {
            .pNext = &rendering_info,
            .flags = vk::PipelineCreateFlags(),
            .stageCount = to_u32(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_info,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = layout,
            .renderPass = nullptr,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        try {
            auto result = device->get_handle().createGraphicsPipeline(nullptr, create_info);
            logi("Successfully created Graphics PipeLine");
            return result.value;
        } catch (vk::SystemError err) {
            loge("Failed to create Graphics Pipeline");
            return nullptr;
        }
    
    }

    vk::PipelineLayout create_pipeline_layout (const vk::DescriptorSetLayout* layout, vk::PushConstantRange* range) {

        auto create_info = vk::PipelineLayoutCreateInfo {
            .flags = vk::PipelineLayoutCreateFlags()
        };

        if (layout != nullptr) {
            create_info.setLayoutCount = 1;
            create_info.pSetLayouts = layout;
        }

        if (range != nullptr) {
            create_info.pushConstantRangeCount = 1;
            create_info.pPushConstantRanges = range;
        }

        try {
            auto result = Device::get()->get_handle().createPipelineLayout(create_info);
            logi("Created PipeLine Layout");
            return result;
        } catch (vk::SystemError) {
            loge("Failed to create PipeLine Layout");
            return nullptr;
        }

    }

    vk::DescriptorSetLayout create_descriptor_set_layout ( ) {

        auto bindings = std::array {
            vk::DescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
            }
        };

        auto descriptor_set_info = vk::DescriptorSetLayoutCreateInfo {
            .flags = vk::DescriptorSetLayoutCreateFlags(),
            .bindingCount = to_u32(bindings.size()),
            .pBindings = bindings.data()
        };

        try {
            auto result = Device::get()->get_handle().createDescriptorSetLayout(descriptor_set_info);
            logi("Created DescriptorSet Pipeline layout");
            return result;
        } catch (vk::SystemError error) {
            loge("Failed to create DescriptorSet Pipeline layout");
            return nullptr;
        }

    }

}