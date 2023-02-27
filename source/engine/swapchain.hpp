#pragma once

#include <vector>

#include "essentials.hpp"

#include "device.hpp"
#include "memory.hpp"

namespace engine {

    class SwapChain {

        struct Frame {
            vk::Image image;
            vk::ImageView view;
            vk::Framebuffer buffer;
            vk::CommandBuffer commands;
            vk::DescriptorSet descriptor_set;

            std::unique_ptr<Buffer> uniform;
            std::unique_ptr<Buffer> storage;

            vk::Semaphore image_available;
            vk::Semaphore render_finished;
            vk::Fence in_flight;
        };

        vk::SwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass renderpass;
        vk::Extent2D extent;

        std::shared_ptr<Device> device;

        void make_frames ( );
        void make_framebuffers ( );

        public:

        SwapChain ( ) = default;
        SwapChain (std::shared_ptr<Device> device, const vk::RenderPass& renderpass)
            : device(device), renderpass(renderpass) { create_handle(); };
        ~SwapChain ( );

        void create_handle ( );

        void make_commandbuffers (vk::CommandPool&);
        void make_descriptor_sets (vk::DescriptorPool&, const vk::DescriptorSetLayout&);

        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle; };
        constexpr std::vector<Frame>& get_frames ( ) { return frames; };
        constexpr const vk::Extent2D& get_extent( ) const { return extent; };

    };

}