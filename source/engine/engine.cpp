#include <limits>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"

#include "core/image.hpp"
#include "core/pipeline.hpp"

#include "utils/utils.hpp"
#include "utils/logging.hpp"
#include "utils/primitives.hpp"

namespace engine {

    Engine::Engine (GLFWwindow* window) {

        logi("Creating Engine instance...");

        if constexpr (debug) {

            auto version = vk::enumerateInstanceVersion();

            auto major = VK_API_VERSION_MAJOR(version);
            auto minor = VK_API_VERSION_MINOR(version);
            auto patch = VK_API_VERSION_PATCH(version);

            logi("System support up to Vulkan {}.{}.{}", major, minor, patch);

        }

        device = std::make_shared<Device>(window);
        Device::set_static_instance(device);

        dldi = vk::DispatchLoaderDynamic(device->get_instance(), vkGetInstanceProcAddr);
        if constexpr (debug) debug_messenger = make_debug_messenger(device->get_instance(), dldi);

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        graphics_queue = device->get_handle().getQueue(indices.graphics_family.value(), 0);
        present_queue = device->get_handle().getQueue(indices.present_family.value(), 0);

        swapchain = std::make_unique<SwapChain>();
        max_frames_in_flight = swapchain->get_frames().size();

        // Layouts
        descriptor_set_layout = create_descriptor_set_layout();

        auto push_constant_range = vk::PushConstantRange {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(UniformBufferObject)
        };

        pipeline_layout = create_pipeline_layout(&descriptor_set_layout, &push_constant_range);

        make_command_pool();
        swapchain->make_commandbuffers(command_pool);
        pipeline = create_pipeline(pipeline_layout);

        if (is_imgui_enabled) ui = std::make_unique<UI>(max_frames_in_flight);

        texture = std::make_unique<Image>("textures/viking_room.png");
        model = std::make_unique<Model>("models/viking_room.obj");

    }

    Engine::~Engine ( ) {

        device->get_handle().waitIdle();

        logi("Destroying Engine...");

        if (debug) device->get_instance().destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        logi("Destroying Command Pool");
        device->get_handle().destroyCommandPool(command_pool);

        logi("Destroying Pipeline");
        device->get_handle().destroyPipelineLayout(pipeline_layout);
        device->get_handle().destroyDescriptorSetLayout(descriptor_set_layout);
        device->get_handle().destroyPipeline(pipeline);

    }

    void Engine::remake_swapchain ( ) {

        SCOPED_PERF_LOG;

        is_framebuffer_resized = false;
        
        auto new_extent = device->get_extent();
        auto old_extent = swapchain->get_extent();

        if (new_extent == old_extent) return;
        device->get_handle().waitIdle();
        swapchain->create_handle();

        frame_number = 0;

    }

    void Engine::make_command_pool ( ) {

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());

        auto create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = indices.graphics_family.value()
        };

        try {
            command_pool = device->get_handle().createCommandPool(create_info);
            logi("Successfully created Command Pool");
        } catch (vk::SystemError err) {
            loge("Failed to create Command Pool");
        }

    }

    void Engine::prepare_frame (uint32_t index) {

        SCOPED_PERF_LOG;

        auto& frame = swapchain->get_frames().at(index);

        static auto start = std::chrono::high_resolution_clock::now();
        auto current = std::chrono::high_resolution_clock::now();

        float delta = std::chrono::duration<float, std::chrono::seconds::period>(start - current).count();

        auto aspect = static_cast<float>(swapchain->get_extent().width) / static_cast<float>(swapchain->get_extent().height);

        auto ubo = UniformBufferObject {
            .model = glm::rotate(glm::mat4(1.0f), delta / 3 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f)
        }; ubo.projection[1][1] *= -1;

        frame.commands.pushConstants(pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ubo), &ubo);

        auto offsets = std::array<vk::DeviceSize, 1> { }; 
        frame.commands.bindVertexBuffers(0, 1, &model->get_vertex(), offsets.data());
        frame.commands.bindIndexBuffer(model->get_index(), 0, vk::IndexType::eUint16);
        frame.commands.drawIndexed(model->get_indices_count(), 1, 0, 0, 0);

    }

    void Engine::record_draw_commands (uint32_t index) {

        SCOPED_PERF_LOG;

        auto& frame = swapchain->get_frames().at(index);
        auto& command_buffer = frame.commands;

        try {
            command_buffer.begin(vk::CommandBufferBeginInfo());
        } catch (vk::SystemError err) {
            loge("Failed to begin command record");
        }

        insert_image_memory_barrier(command_buffer, frame.image, vk::ImageAspectFlagBits::eColor, 
            { vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput },
            { vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite },
            { vk::ImageLayout::eUndefined,  vk::ImageLayout::eColorAttachmentOptimal }
        );

        insert_image_memory_barrier(command_buffer, frame.depth_buffer->get_handle(), vk::ImageAspectFlagBits::eDepth, 
            { vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests },
            { vk::AccessFlagBits::eNone, vk::AccessFlagBits::eDepthStencilAttachmentWrite },
            { vk::ImageLayout::eUndefined,  vk::ImageLayout::eDepthStencilAttachmentOptimal }
        );

        auto color_attachment_info = vk::RenderingAttachmentInfoKHR {
            .imageView = frame.view.get(),
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .resolveMode = vk::ResolveModeFlagBits::eNone,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearValue { std::array { .1f, .1f, .1f, 1.f } }
        };

        auto depth_attachment_info = vk::RenderingAttachmentInfoKHR {
            .imageView = frame.depth_buffer->get_view(),
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .resolveMode = vk::ResolveModeFlagBits::eNone,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eDontCare,
            .clearValue = vk::ClearValue { .depthStencil = { 1.f, 0 } }
        };

        auto rendering_info = vk::RenderingInfo {
            .flags = vk::RenderingFlags(),
            .renderArea = vk::Rect2D { .extent = swapchain->get_extent() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
        };

        command_buffer.beginRenderingKHR(rendering_info, dldi);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &texture->get_descriptor_set(), 0, nullptr);

        auto viewport = vk::Viewport {
            .width = static_cast<float>(swapchain->get_extent().width),
            .height = static_cast<float>(swapchain->get_extent().height),       
            .minDepth = 0.f,
            .maxDepth = 1.f
        };

        command_buffer.setViewport(0, 1, &viewport);

        auto scissor = vk::Rect2D {
            .extent = swapchain->get_extent()
        };

        command_buffer.setScissor(0, 1, &scissor);

        prepare_frame(index);

        if (is_imgui_enabled && on_ui_update) {
            UI::new_frame(); on_ui_update();
            ui->draw(command_buffer, index);
            UI::end_frame();
        }

        command_buffer.endRenderingKHR(dldi);

        insert_image_memory_barrier(command_buffer, frame.image, vk::ImageAspectFlagBits::eColor, 
            { vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eBottomOfPipe },
            { vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eNone },
            { vk::ImageLayout::eColorAttachmentOptimal,  vk::ImageLayout::ePresentSrcKHR }
        );

        try {
            command_buffer.end();
        } catch (vk::SystemError err) {
            loge("Failed to record command buffer");
        }

    }

    void Engine::draw () {

        SCOPED_PERF_LOG;

        if (is_framebuffer_resized) return remake_swapchain();

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        const auto& frame = swapchain->get_frames().at(frame_number);

        if (device->get_handle().waitForFences(frame.in_flight.get(), VK_TRUE, timeout) != vk::Result::eSuccess)
            logw("Something goes wrong when waiting on fences");
        device->get_handle().resetFences(frame.in_flight.get());

        auto image_result = device->get_handle().acquireNextImageKHR(swapchain->get_handle(), timeout, frame.image_available.get());

        frame.commands.reset();

        record_draw_commands(frame_number);
        
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        auto submit_info = vk::SubmitInfo {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.image_available.get(),
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame.commands,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &frame.render_finished.get()
        };

        try {
            graphics_queue.submit(submit_info, frame.in_flight.get());
        } catch (vk::SystemError err) {
            loge("Failed to submit draw command buffer");
        }

        auto present_info = vk::PresentInfoKHR {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.render_finished.get(),
            .swapchainCount = 1,
            .pSwapchains = &swapchain->get_handle(),
            .pImageIndices = &image_result.value
        };

        if (present_queue.presentKHR(present_info) != vk::Result::eSuccess)
            logw("Failed to present image");

        frame_number = (frame_number + 1) % max_frames_in_flight;
    }

}