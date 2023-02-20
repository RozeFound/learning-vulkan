#include <glm/glm.hpp>

#include "pipeline.hpp"
#include "shaders.hpp"
#include "logging.hpp"

namespace engine {

    PipeLine::PipeLine (vk::Device& device, const vk::SurfaceFormatKHR& format)
        : device(device), format(format) {

        auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {
            .flags = vk::PipelineVertexInputStateCreateFlags(),
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
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

        auto shader = Shader(device, "shaders/basic");
        auto stages = shader.get_stage_info(); 

        create_layout();
        create_renderpass();

        auto create_info = vk::GraphicsPipelineCreateInfo {
            .flags = vk::PipelineCreateFlags(),
            .stageCount = static_cast<uint32_t>(stages.size()),
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
            handle = device.createGraphicsPipeline(nullptr, create_info).value;
            LOG_INFO("Successfully created Graphics PipeLine");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Graphics Pipeline");
        }

        shader.destroy();
    
    }

    void PipeLine::create_layout ( ) {

        auto pushconstant_info = vk::PushConstantRange {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .size = sizeof(glm::mat4x4)
        };

        auto create_info = vk::PipelineLayoutCreateInfo {
            .flags = vk::PipelineLayoutCreateFlags(),
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushconstant_info
        };

        try {
            layout = device.createPipelineLayout(create_info);
            LOG_INFO("Created PipeLine Layout");
        } catch (vk::SystemError) {
            LOG_ERROR("Failed to create PipeLine Layout");
        }

    }

    void PipeLine::create_renderpass ( ) {

        auto attachment = vk::AttachmentDescription {
            .flags = vk::AttachmentDescriptionFlags(),
            .format = format.format,
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
            renderpass = device.createRenderPass(create_info);
            LOG_INFO("Created PipeLine renderpass");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create PipeLine renderpass");
        }

    }

    void PipeLine::destroy ( ) {

        LOG_INFO("Destroying Pipeline");
        device.destroyRenderPass(renderpass);
        device.destroyPipelineLayout(layout);
        device.destroyPipeline(handle);

    }

}