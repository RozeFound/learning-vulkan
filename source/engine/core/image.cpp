#include <stdexcept>
#include <vector>

#include "stb_image.h"

#include "image.hpp"

#include "memory.hpp"
#include "pipeline.hpp"

#include "../utils/utils.hpp"
#include "../utils/logging.hpp"

namespace engine {

    auto create_image (std::size_t width, std::size_t height, vk::Format format, vk::ImageUsageFlags usage) {

        auto device = Device::get();
        vk::Image handle; VmaAllocation allocation;
        
        auto create_info = vk::ImageCreateInfo {
            .flags = vk::ImageCreateFlags(),
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {to_u32(width), to_u32(height), 1},
            .mipLevels = 1,
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

            auto requirements = device->get_handle().getImageMemoryRequirements(handle);
            auto index = get_memory_index(requirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

            logi("Successfully created Image");

        } catch (vk::SystemError error) {
            loge("Failed to create Image");
        }

        return std::pair(handle, allocation);

    }

    vk::UniqueImageView create_view (vk::Image& image, vk::Format format, vk::ImageAspectFlags flags) {

        auto subres_range = vk::ImageSubresourceRange {
            .aspectMask = flags,
            .baseMipLevel = 0,
            .levelCount = 1,
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
        create_sampler();
        create_descriptor_set();

        set_data(std::vector<std::byte>(pixels, pixels + size));

        stbi_image_free(pixels);

    }

    Image::Image (std::size_t width, std::size_t height, const std::vector<std::byte>& pixels)
        : width(width), height(height) { 

        size = pixels.size();

        create_handle();
        create_sampler();
        create_descriptor_set();

        set_data(pixels);
    };

    Image::~Image ( ) {

        device->get_handle().destroySampler(sampler);
        vmaDestroyImage(device->get_allocator(), VkImage(handle), allocation);

    }
    
    void Image::create_handle ( ) {

        std::tie(handle, allocation) = create_image(width, height, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
        view = create_view(handle, format, vk::ImageAspectFlagBits::eColor);
        
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
            .maxLod = 0.f,
            .borderColor = vk::BorderColor::eFloatOpaqueBlack,
            .unnormalizedCoordinates = VK_FALSE
        };

        sampler = device->get_handle().createSampler(create_info);

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
            .sampler = sampler,
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

    void Image::set_data(const std::vector<std::byte>& pixels) {

        auto staging = Buffer(size, vk::BufferUsageFlagBits::eTransferSrc);
        staging.write(pixels.data());

        auto transient_buffer = TransientBuffer();
        auto command_buffer = transient_buffer.get();

        insert_image_memory_barrier(command_buffer, handle, vk::ImageAspectFlagBits::eColor, 
            { vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer },
            { vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite },
            { vk::ImageLayout::eUndefined,  vk::ImageLayout::eTransferDstOptimal }
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

        insert_image_memory_barrier(command_buffer, handle, vk::ImageAspectFlagBits::eColor, 
            { vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader },
            { vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead },
            { vk::ImageLayout::eTransferDstOptimal,  vk::ImageLayout::eShaderReadOnlyOptimal }
        );

        transient_buffer.submit();

    }

    DepthImage::DepthImage (std::size_t width, std::size_t height) : width(width), height(height) {

        std::tie(handle, allocation) = create_image(width, height, find_supported_format(), vk::ImageUsageFlagBits::eDepthStencilAttachment);
        view = create_view(handle, find_supported_format(), vk::ImageAspectFlagBits::eDepth);

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