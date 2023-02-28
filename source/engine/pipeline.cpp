#include <glm/glm.hpp>

#include "device.hpp"
#include "pipeline.hpp"
#include "shaders.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    vk::Pipeline create_pipeline (std::shared_ptr<Device> device, vk::PipelineLayout& layout, vk::RenderPass& renderpass) {

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
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f
        };

        auto multisampling = vk::PipelineMultisampleStateCreateInfo {
            .flags = vk::PipelineMultisampleStateCreateFlags(),
            .sampleShadingEnable = VK_FALSE,
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
            .logicOpEnable = VK_FALSE,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment
        };

        auto shader = Shader(device->get_handle(), "shaders/basic");
        auto stages = shader.get_stage_info(); 

        auto create_info = vk::GraphicsPipelineCreateInfo {
            .flags = vk::PipelineCreateFlags(),
            .stageCount = to_u32(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_info,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = layout,
            .renderPass = renderpass,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        try {
            auto result = device->get_handle().createGraphicsPipeline(nullptr, create_info);
            LOG_INFO("Successfully created Graphics PipeLine");
            return result.value;
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Graphics Pipeline");
            return nullptr;
        }

        
    
    }

    vk::PipelineLayout create_pipeline_layout (std::shared_ptr<Device> device, const vk::DescriptorSetLayout& layout) {

        auto create_info = vk::PipelineLayoutCreateInfo {
            .flags = vk::PipelineLayoutCreateFlags(),
            .setLayoutCount = 1,
            .pSetLayouts = &layout
        };

        try {
            auto result = device->get_handle().createPipelineLayout(create_info);
            LOG_INFO("Created PipeLine Layout");
            return result;
        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create PipeLine Layout");
            return nullptr;
        }

    }

    vk::DescriptorSetLayout create_descriptor_set_layout (std::shared_ptr<Device> device) {

        auto bindings = std::array {
            vk::DescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
            },
            vk::DescriptorSetLayoutBinding {
                .binding = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex,
            },
            vk::DescriptorSetLayoutBinding {
                .binding = 2,
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
            auto result = device->get_handle().createDescriptorSetLayout(descriptor_set_info);
            LOG_INFO("Created DescriptorSet Pipeline layout");
            return result;
        } catch (vk::SystemError error) {
            LOG_ERROR("Failed to create DescriptorSet Pipeline layout");
            return nullptr;
        }

    }


    vk::RenderPass create_renderpass (std::shared_ptr<Device> device) {

        auto attachment = vk::AttachmentDescription {
            .flags = vk::AttachmentDescriptionFlags(),
            .format = device->get_format().format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };

        auto attachment_reference = vk::AttachmentReference {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        auto subpass = vk::SubpassDescription {
            .flags = vk::SubpassDescriptionFlags(),
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachment_reference
        };

        auto subpass_dependency = vk::SubpassDependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
        };

        auto create_info = vk::RenderPassCreateInfo {
            .flags = vk::RenderPassCreateFlags(),
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &subpass_dependency
        };

        try {
            auto result = device->get_handle().createRenderPass(create_info);
            LOG_INFO("Created PipeLine renderpass");
            return result;
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create PipeLine renderpass");
            return nullptr;
        }

    }

}