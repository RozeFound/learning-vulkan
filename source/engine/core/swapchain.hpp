#pragma once

#include <memory>
#include <vector>

#include "device.hpp"
#include "image.hpp"
#include "memory.hpp"

namespace engine {

    class SwapChain {

        struct Frame {

            vk::Image image;
            vk::UniqueImageView view;
            vk::CommandBuffer commands;
            vk::UniqueFramebuffer buffer;

            std::shared_ptr<Image> depth_buffer;
            std::shared_ptr<Image> color_buffer;

            vk::UniqueSemaphore image_available;
            vk::UniqueSemaphore render_finished;
            vk::UniqueFence in_flight;

        };

        std::shared_ptr<Image> depth_buffer;
        std::shared_ptr<Image> color_buffer;

        vk::UniqueSwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass render_pass;
        vk::Extent2D extent;

        std::shared_ptr<Device> device = Device::get();

        void make_frames ( );

        public:

        SwapChain (vk::RenderPass render_pass) : render_pass(render_pass) { create_handle(); }

        void create_handle ( );

        void make_commandbuffers (vk::CommandPool&);

        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle.get(); }
        constexpr std::vector<Frame>& get_frames ( ) { return frames; }
        constexpr const vk::Extent2D& get_extent ( ) const { return extent; }

    };

}