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

        auto ec = glz::read_file(settings, "engine_settings.json");

        dldi = vk::DispatchLoaderDynamic(device->get_instance(), vkGetInstanceProcAddr);
        if constexpr (debug) debug_messenger = make_debug_messenger(device->get_instance(), dldi);

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        queue = device->get_handle().getQueue(indices.graphics_family.value(), 0);

        render_pass = create_render_pass();
        swapchain = std::make_unique<SwapChain>(render_pass);
        max_frames_in_flight = swapchain->get_frames().size();

        // Layouts
        descriptor_set_layout = create_descriptor_set_layout();

        auto push_constant_range = vk::PushConstantRange {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(UniformBufferObject)
        };

        auto sample_count = get_max_sample_count(device->get_gpu());
        pipeline_layout = create_pipeline_layout(&descriptor_set_layout, &push_constant_range);
        pipeline = create_pipeline({
            .multisampling_info = create_multisampling_info(sample_count, true),
            .layout = pipeline_layout,
            .render_pass = render_pass,
            .shader_path = "shaders/basic",
        });

        make_command_pool();
        make_command_buffers();

        if (is_imgui_enabled) ui = std::make_unique<UI>(max_frames_in_flight, render_pass);

    }

    Engine::~Engine ( ) {

        auto ec = glz::write_file(settings, "engine_settings.json");

        device->get_handle().waitIdle();

        logi("Destroying Engine...");

        if (debug) device->get_instance().destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        logi("Destroying Command Pool");
        device->get_handle().destroyCommandPool(command_pool);

        logi("Destroying Pipeline");
        device->get_handle().destroyPipelineLayout(pipeline_layout);
        device->get_handle().destroyDescriptorSetLayout(descriptor_set_layout);
        device->get_handle().destroyPipeline(pipeline);
        device->get_handle().destroyRenderPass(render_pass);

    }

    void Engine::apply_settings ( ) {

        if (settings.vsync != swapchain->vsync_enabled) {
            swapchain->vsync_enabled = settings.vsync;
            device->get_handle().waitIdle();
            swapchain->create_handle();
            current_frame = 0;
        }

        fps_limiter.is_enabled = settings.fps_limit == -1 ? false : true;
        if (settings.vsync) fps_limiter.is_enabled = false;

        if (fps_limiter.is_enabled)
            fps_limiter.set_target(settings.fps_limit);

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

    void Engine::make_command_buffers ( ) {

        auto& frames = swapchain->get_frames();
        
        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = to_u32(frames.size())
        };

        try {
            auto buffers = device->get_handle().allocateCommandBuffers(allocate_info);
            for (std::size_t i = 0; i < frames.size(); i++)
                frames.at(i).commands = buffers.at(i);
            logi("Allocated Command Buffers");
        } catch (vk::SystemError err) {
            loge("Failed to allocate Command Buffers");
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
            .view = glm::lookAt(glm::vec3(2.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.3f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .projection = glm::perspective(glm::radians(45.0f), aspect, .1f, 10.0f)
        }; ubo.projection[1][1] *= -1;

        frame.commands.pushConstants(pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ubo), &ubo);

    }

    void Engine::record_draw_commands (uint32_t index, std::shared_ptr<Object> object) {

        SCOPED_PERF_LOG;

        const auto& frame = swapchain->get_frames().at(index);

        try {
            frame.commands.begin(vk::CommandBufferBeginInfo());
        } catch (vk::SystemError err) {
            loge("Failed to begin command record");
        }

        auto clear_values = std::array {
            vk::ClearValue { std::array { .1f, .1f, .1f, 1.f } },
            vk::ClearValue { },
            vk::ClearValue { .depthStencil = { 1.f, 0 } },
        };

        auto renderpass_info = vk::RenderPassBeginInfo {
            .renderPass = render_pass,
            .framebuffer = frame.buffer.get(),
            .renderArea = {{0, 0}, swapchain->get_extent()},
            .clearValueCount = to_u32(clear_values.size()),
            .pClearValues = clear_values.data()
        };

        frame.commands.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);

        auto viewport = vk::Viewport {
            .width = static_cast<float>(swapchain->get_extent().width),
            .height = static_cast<float>(swapchain->get_extent().height),       
            .minDepth = 0.f,
            .maxDepth = 1.f
        };
        frame.commands.setViewport(0, 1, &viewport);

        auto scissor = vk::Rect2D { .extent = swapchain->get_extent() };
        frame.commands.setScissor(0, 1, &scissor);

        prepare_frame(index);

        object->bind(frame.commands, pipeline, pipeline_layout);
        object->draw(frame.commands);

        if (is_imgui_enabled && settings.gui_visible) {
            UI::new_frame();
            if (ui_callback) ui_callback();
            ui->draw(frame.commands, index);
            UI::end_frame();
        }

        frame.commands.endRenderPass();

        try {
            frame.commands.end();
        } catch (vk::SystemError err) {
            loge("Failed to record command buffer");
        }

    }

    void Engine::draw (std::shared_ptr<Object> object) {

        fps_limiter.delay();

        SCOPED_PERF_LOG;

        if (is_framebuffer_resized) {
            swapchain->resize_if_needed();
            is_framebuffer_resized = false;
            current_frame = 0;
        }

        if (is_settings_changed) {
            apply_settings();
            is_settings_changed = false;
        }

        constexpr auto timeout = std::numeric_limits<uint64_t>::max();
        auto& frame = swapchain->get_frames().at(current_frame);
        auto image_index = swapchain->acquire_image(current_frame);
        frame.index = image_index;

        frame.commands.reset();
        record_draw_commands(current_frame, object);
        
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
            queue.submit(submit_info, frame.in_flight.get());
        } catch (vk::SystemError err) {
            loge("Failed to submit draw command buffer");
        }

        if (!swapchain->present_image(current_frame))
            { current_frame = 0; return; }

        current_frame = (current_frame + 1) % max_frames_in_flight;

    }

}