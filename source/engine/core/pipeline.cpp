#include "pipeline.hpp"

#include "device.hpp"
#include "image.hpp"

#include "../utils/logging.hpp"
#include "../utils/utils.hpp"

namespace engine {

    vk::PipelineRasterizationStateCreateInfo create_rasterization_info (vk::CullModeFlags cull_mode) {

        auto rasterization_info = vk::PipelineRasterizationStateCreateInfo {
            .flags = vk::PipelineRasterizationStateCreateFlags(),
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = cull_mode,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f
        };

        return rasterization_info;

    }

    vk::PipelineMultisampleStateCreateInfo create_multisampling_info
        (vk::SampleCountFlagBits sample_count, bool sample_shading) {

        auto multisampling_info = vk::PipelineMultisampleStateCreateInfo {
            .flags = vk::PipelineMultisampleStateCreateFlags(),
            .rasterizationSamples = sample_count,
            .sampleShadingEnable = vk::Bool32(sample_shading)
        };

        return multisampling_info;

    }

    vk::PipelineDepthStencilStateCreateInfo create_depth_stencil_info 
        (bool depth_test, bool depth_write, bool stencil_test) {

        auto depth_stencil_info = vk::PipelineDepthStencilStateCreateInfo {
            .depthTestEnable = vk::Bool32(depth_test),
            .depthWriteEnable = vk::Bool32(depth_write),
            .depthCompareOp = vk::CompareOp::eLess,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = vk::Bool32(stencil_test)
        };

        return  depth_stencil_info;

    }

    vk::PipelineColorBlendAttachmentState create_color_blend_attachment (bool enable_blend,
        std::array<vk::BlendFactor, 2> color_blend_factor, vk::BlendOp color_blend_op,
        std::array<vk::BlendFactor, 2> alpha_blend_factor, vk::BlendOp alpha_blend_op
    ) {

        auto color_blend_attachment = vk::PipelineColorBlendAttachmentState {
            .blendEnable = vk::Bool32(enable_blend),
            .srcColorBlendFactor = color_blend_factor.at(0),
            .dstColorBlendFactor = color_blend_factor.at(1),
            .colorBlendOp = color_blend_op,
            .srcAlphaBlendFactor = alpha_blend_factor.at(0),
            .dstAlphaBlendFactor = alpha_blend_factor.at(1),
            .alphaBlendOp = alpha_blend_op,
            .colorWriteMask = vk::ColorComponentFlagBits::eR 
                            | vk::ColorComponentFlagBits::eG 
                            | vk::ColorComponentFlagBits::eB 
                            | vk::ColorComponentFlagBits::eA
        };

        return color_blend_attachment;

    }

    vk::Pipeline create_pipeline (const PipeLineCreateInfo& create_info) {

        auto device = Device::get();

        auto binding_description = create_info.binding_description;
        auto attribute_descriptions = create_info.attribute_descriptions; 

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

        auto color_blend_info = vk::PipelineColorBlendStateCreateInfo {
            .flags = vk::PipelineColorBlendStateCreateFlags(),
            .attachmentCount = 1,
            .pAttachments = &create_info.color_blend_attachment
        };

        auto color_attachment_format = device->get_format().format;
        auto rendering_info = vk::PipelineRenderingCreateInfoKHR {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &color_attachment_format,
            .depthAttachmentFormat = Image::get_depth_format()
        };

        auto shader = Shader(create_info.shader_path);
        auto stages = shader.get_stage_info(); 

        auto pipeline_create_info = vk::GraphicsPipelineCreateInfo {
            .pNext = create_info.render_pass ? nullptr : &rendering_info,
            .flags = vk::PipelineCreateFlags(),
            .stageCount = to_u32(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_info,
            .pRasterizationState = &create_info.rasterization_info,
            .pMultisampleState = &create_info.multisampling_info,
            .pDepthStencilState = &create_info.depth_stencil_info,
            .pColorBlendState = &color_blend_info,
            .pDynamicState = &dynamic_state_info,
            .layout = create_info.layout,
            .renderPass = create_info.render_pass,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
        };

        try {
            auto result = device->get_handle().createGraphicsPipeline(nullptr, pipeline_create_info);
            logi("Successfully created Graphics PipeLine");
            return result.value;
        } catch (vk::SystemError err) {
            loge("Failed to create Graphics Pipeline");
            return nullptr;
        }

    }

    vk::RenderPass create_render_pass ( ) {

        auto device = Device::get();
        auto sample_count = get_max_sample_count(device->get_gpu());

        auto color_attachment = vk::AttachmentDescription {
            .flags = vk::AttachmentDescriptionFlags(),
            .format = device->get_format().format,
            .samples = sample_count,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
        };

        auto color_attachment_reference = vk::AttachmentReference {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        auto resolve_attachment = vk::AttachmentDescription {
            .flags = vk::AttachmentDescriptionFlags(),
            .format = device->get_format().format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eDontCare,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };

        auto resolve_attachment_reference = vk::AttachmentReference {
            .attachment = 1,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        auto depth_attachment = vk::AttachmentDescription {
            .flags = vk::AttachmentDescriptionFlags(),
            .format = Image::get_depth_format(),
            .samples = sample_count,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eDontCare,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        auto depth_attachment_reference = vk::AttachmentReference {
            .attachment = 2,
            .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        auto subpass = vk::SubpassDescription {
            .flags = vk::SubpassDescriptionFlags(),
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_reference,
            .pResolveAttachments = &resolve_attachment_reference,
            .pDepthStencilAttachment = &depth_attachment_reference
        };

        auto subpass_dependencies = std::array {
            vk::SubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests,
                .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
                .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead
            },
            vk::SubpassDependency {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead
            }
        };

        auto attachments = std::array { color_attachment, resolve_attachment, depth_attachment };

        auto create_info = vk::RenderPassCreateInfo {
            .flags = vk::RenderPassCreateFlags(),
            .attachmentCount = to_u32(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = to_u32(subpass_dependencies.size()),
            .pDependencies = subpass_dependencies.data()
        };

        try {
            auto result = device->get_handle().createRenderPass(create_info);
            logi("Created PipeLine renderpass");
            return result;
        } catch (vk::SystemError err) {
            loge("Failed to create PipeLine renderpass");
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