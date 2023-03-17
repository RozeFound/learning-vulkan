#include <stdexcept>
#include <vector>

#include "stb_image.h"

#include "image.hpp"

#include "memory.hpp"
#include "pipeline.hpp"

#include "../utils/utils.hpp"
#include "../utils/logging.hpp"

namespace engine {

    auto create_image (std::size_t width, std::size_t height, uint32_t mip_levels, vk::Format format, vk::ImageUsageFlags usage) {

        auto device = Device::get();
        vk::Image handle; VmaAllocation allocation;
        
        auto create_info = vk::ImageCreateInfo {
            .flags = vk::ImageCreateFlags(),
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {to_u32(width), to_u32(height), 1},
            .mipLevels = mip_levels,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        try {

            auto allocation_info = VmaAllocationCreateInfo {
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            vmaCreateImage(device->get_allocator(), reinterpret_cast<VkImageCreateInfo*>(&create_info),
                &allocation_info, reinterpret_cast<VkImage*>(&handle), &allocation, nullptr);

            logi("Successfully created Image");

        } catch (vk::SystemError error) {
            loge("Failed to create Image");
        }

        return std::pair(handle, allocation);

    }

    vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags, uint32_t mip_levels) {

        auto subres_range = vk::ImageSubresourceRange {
            .aspectMask = flags,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        auto create_info = vk::ImageViewCreateInfo {
            .flags = vk::ImageViewCreateFlags(),
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .components = vk::ComponentMapping(),
            .subresourceRange = subres_range
        };

        return Device::get()->get_handle().createImageViewUnique(create_info);

    }
    
    Image::Image (std::string_view path) {

        int width, height, channels;

        auto stbi_image = stbi_load(path.data(), &width, &height, &channels, 4);
        auto pixels = reinterpret_cast<std::byte*>(stbi_image);

        this->width = width;
        this->height = height;
        size = width * height * 4; 

        create_handle();
        set_data({pixels, pixels + size});
        stbi_image_free(pixels);

    }

    Image::Image (std::size_t width, std::size_t height, std::span<std::byte> pixels)
        : width(width), height(height) { 

        size = pixels.size();
        create_handle();
        set_data(pixels);

    };

    Image::~Image ( ) {

        vmaDestroyImage(device->get_allocator(), VkImage(handle), allocation);

    }
    
    void Image::create_handle ( ) {

        mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        using enum vk::ImageUsageFlagBits;

        std::tie(handle, allocation) = create_image(width, height, mip_levels, format, eTransferSrc | eTransferDst | eSampled);
        view = create_view(handle, format, vk::ImageAspectFlagBits::eColor, mip_levels);

        create_sampler();
        create_descriptor_set();
        
    }

    void Image::create_sampler ( ) {

        auto properties = device->get_gpu().getProperties();

        auto create_info = vk::SamplerCreateInfo {
            .flags = vk::SamplerCreateFlags(),
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = -0.f,
            .maxLod = static_cast<float>(mip_levels),
            .borderColor = vk::BorderColor::eFloatOpaqueBlack,
            .unnormalizedCoordinates = VK_FALSE
        };

        sampler = device->get_handle().createSamplerUnique(create_info);

    }

    void Image::create_descriptor_set ( ) {

        auto pool_size = vk::DescriptorPoolSize {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1
        };

        auto create_info = vk::DescriptorPoolCreateInfo {
            .flags = vk::DescriptorPoolCreateFlags(),
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size
        };

        try {
            descriptor_pool = device->get_handle().createDescriptorPoolUnique(create_info);
            logi("Successfully created Descriptor Pool");
        } catch (vk::SystemError err) {
            loge("Failed to create Descriptor Pool");
        }

        auto layout = create_descriptor_set_layout();

        auto allocate_info = vk::DescriptorSetAllocateInfo {
            .descriptorPool = descriptor_pool.get(),
            .descriptorSetCount = 1,
            .pSetLayouts = &layout
        };

        try {
            auto result = device->get_handle().allocateDescriptorSets(allocate_info);
            logi("Allocated DescriptorSet's");
            descriptor_set = result.at(0);
        } catch (vk::SystemError err) {
            loge("Failed to allocate DescriptorSet's");
        }

        auto image_info = vk::DescriptorImageInfo {
            .sampler = sampler.get(),
            .imageView = view.get(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        auto write_info = vk::WriteDescriptorSet {
            .dstSet = descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };

        device->get_handle().updateDescriptorSets(1, &write_info, 0, nullptr);
        device->get_handle().destroyDescriptorSetLayout(layout);

    }

    void Image::generate_mipmaps (const vk::CommandBuffer& command_buffer) {

        auto barrier = vk::ImageMemoryBarrier {
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = handle,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        int32_t mip_width = std::make_signed_t<uint32_t>(to_u32(width));
        int32_t mip_height = std::make_signed_t<uint32_t>(to_u32(height));

        for (uint32_t i = 1; i < mip_levels; i++) {

            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;

            insert_image_memory_barrier(command_buffer, barrier, 
                { vk::PipelineStageFlagBits::eTransfer, 
                    vk::PipelineStageFlagBits::eTransfer }
            );

            auto blit = vk::ImageBlit {
                .srcSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .srcOffsets = { { 0, 0, 0, mip_width, mip_height, 1 } },
                .dstSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .dstOffsets ={ { 0, 0, 0, mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 } }
            };

            command_buffer.blitImage(
                handle, vk::ImageLayout::eTransferSrcOptimal,
                handle, vk::ImageLayout::eTransferDstOptimal,
                1, &blit, vk::Filter::eLinear
            );

            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

            insert_image_memory_barrier(command_buffer, barrier,
                { vk::PipelineStageFlagBits::eTransfer, 
                    vk::PipelineStageFlagBits::eFragmentShader }
            );

            if (mip_width > 1) mip_width /= 2;
            if (mip_height > 1) mip_height /= 2;

        }

        barrier.subresourceRange.baseMipLevel = mip_levels - 1;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        insert_image_memory_barrier(command_buffer, barrier,
            { vk::PipelineStageFlagBits::eTransfer, 
                vk::PipelineStageFlagBits::eFragmentShader }
        );

    }

    void Image::set_data(std::span<std::byte> pixels) {

        auto staging = Buffer(size, vk::BufferUsageFlagBits::eTransferSrc);
        staging.write(pixels.data());

        auto transient_buffer = TransientBuffer(true);
        auto command_buffer = transient_buffer.get();

        insert_image_memory_barrier(command_buffer, handle, vk::ImageAspectFlagBits::eColor, 
            { vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer },
            { vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite },
            { vk::ImageLayout::eUndefined,  vk::ImageLayout::eTransferDstOptimal }, mip_levels
        );

        auto subres_layers = vk::ImageSubresourceLayers {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        auto region = vk::BufferImageCopy {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = subres_layers,
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { to_u32(width), to_u32(height), 1 }
        };

        command_buffer.copyBufferToImage(staging.get_handle(), handle,
            vk::ImageLayout::eTransferDstOptimal, 1, &region);

        generate_mipmaps(command_buffer);

        transient_buffer.submit();

    }

    DepthImage::DepthImage (std::size_t width, std::size_t height) : width(width), height(height) {

        std::tie(handle, allocation) = create_image(width, height, 1, format, vk::ImageUsageFlagBits::eDepthStencilAttachment);
        view = create_view(handle, format, vk::ImageAspectFlagBits::eDepth);

    }

    DepthImage::~DepthImage ( ) {

        vmaDestroyImage(device->get_allocator(), VkImage(handle), allocation);

    }

    vk::Format DepthImage::find_supported_format () {

        auto candidates = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };

        for (const auto& format : candidates) {

            auto properties = Device::get()->get_gpu().getFormatProperties(format);

            if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                return format;
        }

        throw std::runtime_error("Failed to retrieve Depth Format");

    }

}