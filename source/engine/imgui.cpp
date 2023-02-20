#include <imgui/roboto_regular.h>

#include "imgui.hpp"
#include "utils.hpp"
#include "logging.hpp"

namespace engine {

    ImGUI::ImGUI (vk::PhysicalDevice& physical_device, vk::Device& device, vk::Instance& instance, vk::SurfaceKHR& surface, SwapChain& swapchain, PipeLine& pipeline, GLFWwindow* window)
        : physical_device(physical_device), device(device), instance(instance), surface(surface), swapchain(swapchain), pipeline(pipeline), window(window) {

        image_count = swapchain.get_frames().size();
 
        make_command_pool();
        make_descriptor_pool();

        make_framebuffers();
        make_command_buffers();

        init_imgui();
        init_font();

    }

    void ImGUI::init_imgui ( ) {

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        auto indices = get_queue_family_indices(physical_device, surface);
        auto graphics_queue = device.getQueue(indices.graphics_family.value(), 0);

        ImGui_ImplGlfw_InitForVulkan(window, true);
        auto init_info = ImGui_ImplVulkan_InitInfo {
            .Instance = instance,
            .PhysicalDevice = physical_device,
            .Device = device,
            .QueueFamily = indices.graphics_family.value(),
            .Queue = graphics_queue,
            .PipelineCache = nullptr,
            .DescriptorPool = descriptor_pool,
            .Subpass = 0,
            .MinImageCount = image_count,
            .ImageCount = image_count,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        };

        ImGui_ImplVulkan_Init(&init_info, pipeline.get_renderpass());
        
    }

    void ImGUI::init_font ( ) {

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto font_config = ImFontConfig();
        font_config.FontDataOwnedByAtlas = false;
        ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)font, 
            sizeof(font), 16.0f, &font_config);
        io.FontDefault = robotoFont;

        const auto& command_buffer = command_buffers.at(1);
        device.resetCommandPool(command_pool);

        auto indices = get_queue_family_indices(physical_device, surface);
        auto graphics_queue = device.getQueue(indices.graphics_family.value(), 0);

        auto begin_info = vk::CommandBufferBeginInfo {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        try {
            command_buffer.begin(begin_info);
            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
            command_buffer.end();

            auto submit_info = vk::SubmitInfo {
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer
            };
            graphics_queue.submit(submit_info);            
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create font texture");
        }

        device.waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();

    }

    void ImGUI::destroy ( ) {

        for (const auto& buffer : frame_buffers)
            device.destroyFramebuffer(buffer);

        device.destroyCommandPool(command_pool);

        LOG_INFO("Destroying ImGUI");
        ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

        LOG_INFO("Destroying Descriptor Pool");
        device.destroyDescriptorPool(descriptor_pool);

    }

    vk::CommandBuffer& ImGUI::get_commands (uint32_t index, std::function<void()> func) {

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto& command_buffer = command_buffers.at(index);

        auto begin_info = vk::CommandBufferBeginInfo();
        try {
            command_buffer.begin(begin_info);
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to begin command record for ImGUI");
        }

        auto clear_color = vk::ClearValue { std::array {.2f, .2f, .2f, 1.f} };

        auto renderpass_info = vk::RenderPassBeginInfo {
            .renderPass = pipeline.get_renderpass(),
            .framebuffer = frame_buffers.at(index),
            .renderArea = {{0, 0}, swapchain.get_extent()},
            .clearValueCount = 1,
            .pClearValues = &clear_color
        };

        command_buffer.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);

        if (func != nullptr) func();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

        command_buffer.endRenderPass();

        try {
            command_buffer.end();
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to record command buffer for ImGUI");
        }

        return command_buffer;

    }

    void ImGUI::make_command_pool ( ) {

        auto indices = get_queue_family_indices(physical_device, surface);

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = indices.graphics_family.value()
        };

        try {
            command_pool = device.createCommandPool(create_info);
            LOG_INFO("Successfully created Command Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Command Pool");
        }

    }

    void ImGUI::make_descriptor_pool ( ) {

        auto pool_sizes = std::vector<vk::DescriptorPoolSize> {
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

        auto create_info = vk::DescriptorPoolCreateInfo {
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = static_cast<uint32_t>(1000 * pool_sizes.size()),
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data()
        };

        try {
            descriptor_pool = device.createDescriptorPool(create_info);
            LOG_INFO("Successfully created Descriptor Pool");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Descriptor Pool");
        }

    }

    void ImGUI::make_framebuffers ( ) {

        frame_buffers.resize(image_count);

        for (std::size_t i = 0; i < frame_buffers.size(); i++) {

            const auto& frame = swapchain.get_frames().at(i);

            auto attachments = std::vector { frame.view };

            auto create_info = vk::FramebufferCreateInfo {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = pipeline.get_renderpass(),
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = swapchain.get_extent().width,
                .height = swapchain.get_extent().height,
                .layers = 1
            };

            try {
                frame_buffers.at(i) = device.createFramebuffer(create_info);
            } catch (vk::SystemError err) {
                LOG_ERROR("Failed to create Framebuffer");
            }

        }

        LOG_INFO("Created buffers for frames");

    }

    void ImGUI::make_command_buffers ( ) {

        command_buffers.resize(image_count);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = image_count,
        };

        try {
            auto buffers = device.allocateCommandBuffers(allocate_info);
            for (std::size_t i = 0; i < command_buffers.size(); i++)
                command_buffers.at(i) = buffers.at(i);
            LOG_INFO("Allocated Command Buffers for ImGUI");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to allocate Command Buffers for ImGUI");
        }

    }

}