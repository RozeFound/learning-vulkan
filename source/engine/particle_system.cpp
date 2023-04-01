#include <random>

#include "particle_system.hpp"

#include "utils/utils.hpp"
#include "utils/logging.hpp"

namespace engine {

    ParticleSystem::ParticleSystem (uint32_t frames_in_flight, const vk::RenderPass& render_pass)
        : frames_in_flight(frames_in_flight), render_pass(render_pass) {

        fill_particles();
        prepare_buffers();
        
        make_command_pool();
        make_command_buffers();

        make_descriptor_set_layout();
        make_descriptor_set();

        auto push_constant_range = vk::PushConstantRange {
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
            .offset = 0,
            .size = sizeof(float)
        };

        compute_layout = create_pipeline_layout(&descriptor_set_layout.get(), &push_constant_range);
        compute_pipeline = create_compute_pipeline(compute_layout, "shaders/particles");

        graphics_layout = create_pipeline_layout();
        auto sample_count = get_max_sample_count(device->get_gpu());
        graphics_pipeline = create_pipeline({
            .binding_description = Particle::get_binding_description(),
            .attribute_descriptions = Particle::get_attribute_descriptions(),
            .input_assembly_info = create_input_assembly_info(vk::PrimitiveTopology::ePointList),
            .multisampling_info = create_multisampling_info(sample_count, true),
            .depth_stencil_info = create_depth_stencil_info(false, false),
            .color_blend_attachment = create_color_blend_attachment(true,
                { vk::BlendFactor::eSrcAlpha,  vk::BlendFactor::eOneMinusSrcAlpha }, vk::BlendOp::eAdd,
                { vk::BlendFactor::eOneMinusSrcAlpha,  vk::BlendFactor::eDstAlpha }, vk::BlendOp::eAdd
            ),
            .layout = graphics_layout,
            .render_pass = render_pass,
            .shader_path = "shaders/g_particles"
        });

        for (uint32_t i = 0; i < frames_in_flight; i++) {
            fences.push_back(make_fence(device->get_handle()));
            semaphores.push_back(make_semaphore(device->get_handle()));
        }

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        queue = device->get_handle().getQueue(indices.graphics_family.value(), 0);

    }

    ParticleSystem::~ParticleSystem ( ) {

        device->get_handle().destroyPipeline(compute_pipeline);
        device->get_handle().destroyPipelineLayout(compute_layout);

        device->get_handle().destroyPipeline(graphics_pipeline);
        device->get_handle().destroyPipelineLayout(graphics_layout);

        device->get_handle().destroyCommandPool(command_pool);

    }

    void ParticleSystem::fill_particles ( ) {

        auto random_device = std::random_device{}();
        auto engine = std::default_random_engine(random_device);
        auto distribution = std::uniform_real_distribution<float>(0.f, 1.f);
        auto rand_float = std::bind(distribution, engine);

        particles.resize(particles_count);
        auto extent = device->get_extent();

        for (auto& particle : particles) {

            float r = 0.01f * std::sqrt(rand_float());
            float theta = rand_float() * 2 * std::numbers::pi;
            float x = r * std::cos(theta) * extent.height / extent.width;
            float y = r * std::sin(theta);

            particle.position = glm::vec2(x, y);
            particle.velocity = glm::normalize(glm::vec2(x,y)) * 0.00025f;
            particle.color = glm::vec4(rand_float(),rand_float(), rand_float(), 1.0f);

        }
    }

    void ParticleSystem::prepare_buffers ( ) {

        using enum vk::BufferUsageFlagBits;
        auto buffer_size = particles.size() * sizeof(Particle);

        auto staging_buffer = Buffer(buffer_size, eTransferSrc);
        staging_buffer.write(particles.data());

        for (uint32_t i = 0; i < frames_in_flight; i++) {
            auto buffer = std::make_shared<Buffer>(buffer_size, eVertexBuffer | eStorageBuffer, false, true);
            copy_buffer(staging_buffer.get_handle(), buffer->get_handle(), buffer_size);
            buffers.push_back(buffer);
        }

    }

    void ParticleSystem::make_command_pool ( ) {

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

    void ParticleSystem::make_command_buffers ( ) {
       
        auto allocate_info = vk::CommandBufferAllocateInfo {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = frames_in_flight
        };

        try {
            auto buffers = device->get_handle().allocateCommandBuffers(allocate_info);
            for (auto& buffer : buffers) command_buffers.push_back(buffer);
            logi("Allocated Command Buffers");
        } catch (vk::SystemError err) {
            loge("Failed to allocate Command Buffers");
        }

    }

    void ParticleSystem::make_descriptor_set_layout ( ) {

        auto bindings = Particle::get_descriptor_set_layout_bindings();

        auto create_info = vk::DescriptorSetLayoutCreateInfo {
            .flags = vk::DescriptorSetLayoutCreateFlags(),
            .bindingCount = to_u32(bindings.size()),
            .pBindings = bindings.data()
        };

        try {
            descriptor_set_layout = Device::get()->get_handle().createDescriptorSetLayoutUnique(create_info);
            logi("Created DescriptorSet Pipeline layout");
        } catch (vk::SystemError error) {
            loge("Failed to create DescriptorSet layout");
        }

    };

    void ParticleSystem::make_descriptor_set ( ) {

        auto pool_sizes = std::array {
            vk::DescriptorPoolSize {
                .type = vk::DescriptorType::eStorageBuffer,
                .descriptorCount = 2 * frames_in_flight
            }
        };

        auto create_info = vk::DescriptorPoolCreateInfo {
            .flags = vk::DescriptorPoolCreateFlags(),
            .maxSets = frames_in_flight,
            .poolSizeCount = to_u32(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data()
        };

        try {
            descriptor_pool = device->get_handle().createDescriptorPoolUnique(create_info);
            logi("Successfully created Descriptor Pool");
        } catch (vk::SystemError err) {
            loge("Failed to create Descriptor Pool");
        }

        auto layouts = std::array {
            descriptor_set_layout.get(),
            descriptor_set_layout.get(),
            descriptor_set_layout.get()
        };

        auto allocate_info = vk::DescriptorSetAllocateInfo {
            .descriptorPool = descriptor_pool.get(),
            .descriptorSetCount = frames_in_flight,
            .pSetLayouts = layouts.data()
        };

        try {
            descriptor_sets = device->get_handle().allocateDescriptorSets(allocate_info);
            logi("Allocated DescriptorSet's");
        } catch (vk::SystemError err) {
            loge("Failed to allocate DescriptorSet's");
        }

        for (uint32_t i = 0; i < frames_in_flight; i++) {

            auto last_frame_sbo = vk::DescriptorBufferInfo {
                .buffer = buffers.at((i - 1) % frames_in_flight)->get_handle(),
                .offset = 0,
                .range = sizeof(Particle) * particles_count
            };

            auto current_frame_sbo = vk::DescriptorBufferInfo {
                .buffer = buffers.at(i)->get_handle(),
                .offset = 0,
                .range = sizeof(Particle) * particles_count
            };

            auto descriptor_writes = std::array {
                vk::WriteDescriptorSet {
                    .dstSet = descriptor_sets.at(i),
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &last_frame_sbo
                },
                vk::WriteDescriptorSet {
                    .dstSet = descriptor_sets.at(i),
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &current_frame_sbo
                }
            };

            device->get_handle().updateDescriptorSets(to_u32(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);

        }

    }

    void ParticleSystem::record_compute_commands (uint32_t index) {

        auto commands = command_buffers.at(index);
        constexpr auto timeout = std::numeric_limits<uint64_t>::max();

        if (device->get_handle().waitForFences(fences.at(index).get(), VK_TRUE, timeout) != vk::Result::eSuccess)
            logw("Something goes wrong when waiting on fences");

        static auto start = std::chrono::high_resolution_clock::now();
        auto current = std::chrono::high_resolution_clock::now();

        float delta = std::chrono::duration<float, std::chrono::seconds::period>(start - current).count();

        device->get_handle().resetFences(fences.at(index).get());

        commands.reset();

        try {
            commands.begin(vk::CommandBufferBeginInfo());
        } catch (vk::SystemError err) {
            loge("Failed to begin command record");
        }

        commands.pushConstants(compute_layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(float), &delta);

        commands.bindPipeline(vk::PipelineBindPoint::eCompute, compute_pipeline);
        commands.bindDescriptorSets(vk::PipelineBindPoint::eCompute, compute_layout, 0, 1, &descriptor_sets.at(index), 0, nullptr);

        commands.dispatch(particles_count / 256, 1, 1);

        try {
            commands.end();
        } catch (vk::SystemError err) {
            loge("Failed to record command buffer");
        }

    }

    void ParticleSystem::draw (uint32_t index, const vk::CommandBuffer& commands) {

        
        auto offsets = std::array<vk::DeviceSize, 1> { }; 
        commands.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
        commands.bindVertexBuffers(0, 1, &buffers.at(index)->get_handle(), offsets.data());
        commands.draw(particles_count, 1, 0, 0);

    }

    void ParticleSystem::compute_submit (uint32_t index) {

        auto submit_info = vk::SubmitInfo {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffers.at(index),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &semaphores.at(index).get()
        };

        try {
            queue.submit(submit_info, fences.at(index).get());
        } catch (vk::SystemError err) {
            loge("Failed to submit draw command buffer");
        }

    }

}