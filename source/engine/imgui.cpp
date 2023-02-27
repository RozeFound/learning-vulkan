#include <imgui/roboto_regular.h>

#include "imgui.hpp"
#include "utils.hpp"
#include "logging.hpp"

namespace engine {

    ImGUI::ImGUI (std::shared_ptr<Device> device, std::size_t image_count, vk::RenderPass& renderpass)
        : device(device), image_count(to_u32(image_count)), renderpass(renderpass) {

        make_descriptor_pool();

        init_imgui();
        init_font();

    }

    void ImGUI::init_imgui ( ) {

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto graphics_queue = device->get_handle().getQueue(indices.graphics_family.value(), 0);

        ImGui_ImplGlfw_InitForVulkan(const_cast<GLFWwindow*>(device->get_window()), true);
        auto init_info = ImGui_ImplVulkan_InitInfo {
            .Instance = device->get_instance(),
            .PhysicalDevice = device->get_gpu(),
            .Device = device->get_handle(),
            .QueueFamily = indices.graphics_family.value(),
            .Queue = graphics_queue,
            .PipelineCache = nullptr,
            .DescriptorPool = descriptor_pool,
            .Subpass = 0,
            .MinImageCount = image_count,
            .ImageCount = image_count,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        };

        ImGui_ImplVulkan_Init(&init_info, renderpass);
        
    }

    void ImGUI::init_font ( ) {

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto font_config = ImFontConfig();
        font_config.FontDataOwnedByAtlas = false;
        ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)font, 
            sizeof(font), 16.0f, &font_config);
        io.FontDefault = robotoFont;

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eTransient,
            .queueFamilyIndex = indices.transfer_family.value()
        };
        auto command_pool = device->get_handle().createCommandPool(create_info);

        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto command_buffer = device->get_handle().allocateCommandBuffers(allocate_info).at(0);
        auto transit_queue = device->get_handle().getQueue(indices.transfer_family.value(), 0);

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
            transit_queue.submit(submit_info);            
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create font texture");
        }

        device->get_handle().waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
        device->get_handle().destroyCommandPool(command_pool);

    }

    ImGUI::~ImGUI ( ) {

        LOG_INFO("Destroying ImGUI");
        ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

        LOG_INFO("Destroying ImGUI Descriptor Pool");
        device->get_handle().destroyDescriptorPool(descriptor_pool);

    }

    void ImGUI::draw (vk::CommandBuffer& command_buffer, std::function<void()> func) {

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (func != nullptr) func();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

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
            descriptor_pool = device->get_handle().createDescriptorPool(create_info);
            LOG_INFO("Successfully created Descriptor Pool for ImGUI");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to create Descriptor Pool for ImGUI");
        }

    }

}