#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "device.hpp"
#include "image.hpp"
#include "memory.hpp"

namespace engine {

    class SwapChain {

        struct Frame {

            vk::Image image;
            vk::UniqueImageView view;

            std::unique_ptr<DepthImage> depth_buffer;

            vk::CommandBuffer commands;
            vk::DescriptorSet descriptor_set;

            std::shared_ptr<Image> texture;

            vk::UniqueSemaphore image_available;
            vk::UniqueSemaphore render_finished;
            vk::UniqueFence in_flight;

        };

        vk::UniqueSwapchainKHR handle;
        std::vector<Frame> frames;
        vk::RenderPass renderpass;
        vk::Extent2D extent;

        std::unique_ptr<DepthImage> depth_buffer;

        std::shared_ptr<Device> device = Device::get();

        void make_frames ( );

        public:

        SwapChain () { create_handle(); }

        void create_handle ( );

        void make_commandbuffers (vk::CommandPool&);
        void make_descriptor_sets (vk::DescriptorPool&, const vk::DescriptorSetLayout&);

        constexpr const vk::SwapchainKHR& get_handle ( ) const { return handle.get(); }
        constexpr std::vector<Frame>& get_frames ( ) { return frames; }
        constexpr const vk::Extent2D& get_extent( ) const { return extent; }

    };

}