#pragma once

#include "core/memory.hpp"
#include "core/pipeline.hpp"

#include "utils/primitives.hpp"

namespace engine {

    class ParticleSystem {

        uint32_t frames_in_flight;
        const std::size_t particles_count = 4096; 
        std::shared_ptr<Device> device = Device::get();

        std::vector<Particle> particles;
        std::vector<std::shared_ptr<Buffer>> buffers;

        std::vector<vk::UniqueFence> fences;
        std::vector<vk::UniqueSemaphore> semaphores;

        vk::RenderPass render_pass;
        vk::Pipeline graphics_pipeline;
        vk::PipelineLayout graphics_layout;

        vk::Pipeline compute_pipeline;
        vk::PipelineLayout compute_layout;
        vk::Queue queue;

        vk::UniqueDescriptorPool descriptor_pool;
        vk::UniqueDescriptorSetLayout descriptor_set_layout;
        std::vector<vk::DescriptorSet> descriptor_sets;

        vk::CommandPool command_pool;
        std::vector<vk::CommandBuffer> command_buffers;

        void make_descriptor_set_layout ( );
        void make_descriptor_set ( );

        void fill_particles ( );
        void prepare_buffers ( );

        void make_command_pool ( );
        void make_command_buffers ( );

        public:

        ParticleSystem ( ) = default;
        ParticleSystem (uint32_t frames_in_flight, const vk::RenderPass& render_pass);
        ~ParticleSystem ( );

        void record_compute_commands (uint32_t index);
        void draw (uint32_t index, const vk::CommandBuffer& commands);
        void compute_submit (uint32_t index);

        constexpr const vk::Semaphore get_semaphore (uint32_t index) const { return semaphores.at(index).get(); }

    };

}

