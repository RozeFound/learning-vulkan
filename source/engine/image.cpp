#include <stdexcept>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "image.hpp"
#include "memory.hpp"
#include "utils.hpp"
#include "logging.hpp"

namespace engine {
    
    Image::Image (std::string_view path) {

        int width, height, channels;

        auto stbi_image = stbi_load(path.data(), &width, &height, &channels, 4);
        auto pixels = reinterpret_cast<std::byte*>(stbi_image);

        this->width = width;
        this->height = height;
        size = width * height * 4; 

        create_handle();
        create_view();
        create_sampler();

        set_data(std::vector<std::byte>(pixels, pixels + size));

        stbi_image_free(pixels);

    }

    Image::Image (std::size_t width, std::size_t height, std::vector<std::byte> pixels)
        : width(width), height(height) { 

        create_handle();
        create_view();
        create_sampler();

        set_data(pixels);
    };

    Image::~Image ( ) {

        device->get_handle().destroySampler(sampler);
        device->get_handle().destroyImageView(view);
        device->get_handle().destroyImage(handle);
        device->get_handle().freeMemory(memory);

    }

    void Image::create_handle ( ) {

        auto create_info = vk::ImageCreateInfo {
            .flags = vk::ImageCreateFlags(),
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {to_u32(width), to_u32(height), 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        try {
            handle = device->get_handle().createImage(create_info);

            auto requirements = device->get_handle().getImageMemoryRequirements(handle);
            auto index = get_memory_index(requirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = index
            };

            memory = device->get_handle().allocateMemory(allocate_info);
            device->get_handle().bindImageMemory(handle, memory, 0);
            LOG_INFO("Successfully created Image");
        } catch (vk::SystemError error) {
            LOG_ERROR("Failed to create Image");
        }

    }

    void Image::create_view ( ) {

        auto subres_range = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        auto create_info = vk::ImageViewCreateInfo {
            .flags = vk::ImageViewCreateFlags(),
            .image = handle,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .components = vk::ComponentMapping(),
            .subresourceRange = subres_range
        };

        view = device->get_handle().createImageView(create_info);

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
            .minLod = 0.f,
            .maxLod = 0.f,
            .borderColor = vk::BorderColor::eFloatOpaqueBlack,
            .unnormalizedCoordinates = VK_FALSE
        };

        sampler = device->get_handle().createSampler(create_info);

    }

    void Image::set_data(std::vector<std::byte> pixels) {

        auto staging = Buffer(size, vk::BufferUsageFlagBits::eTransferSrc);
        staging.write(pixels.data());

        auto indices = get_queue_family_indices(device->get_gpu(), device->get_surface());
        auto transient_buffer = TransientBuffer(device->get_handle(), indices);
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
            .imageOffset = {0, 0, 0},
            .imageExtent = {to_u32(width), to_u32(height), 1}
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

    DepthImage::DepthImage (std::size_t width, std::size_t height) 
        : width(width), height(height) {

        create_handle();
        create_view();

    }

    void DepthImage::create_handle ( ) {

        auto create_info = vk::ImageCreateInfo {
            .flags = vk::ImageCreateFlags(),
            .imageType = vk::ImageType::e2D,
            .format = find_supported_format(),
            .extent = {to_u32(width), to_u32(height), 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        try {
            handle = device->get_handle().createImageUnique(create_info);

            auto requirements = device->get_handle().getImageMemoryRequirements(handle.get());
            auto index = get_memory_index(requirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

            auto allocate_info = vk::MemoryAllocateInfo {
                .allocationSize = requirements.size,
                .memoryTypeIndex = index
            };

            memory = device->get_handle().allocateMemoryUnique(allocate_info);
            device->get_handle().bindImageMemory(handle.get(), memory.get(), 0);
            LOG_INFO("Successfully created Image");
        } catch (vk::SystemError error) {
            LOG_ERROR("Failed to create Image");
        }

    }

    void DepthImage::create_view ( ) {

        auto subres_range = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        auto create_info = vk::ImageViewCreateInfo {
            .flags = vk::ImageViewCreateFlags(),
            .image = handle.get(),
            .viewType = vk::ImageViewType::e2D,
            .format = find_supported_format(),
            .components = vk::ComponentMapping(),
            .subresourceRange = subres_range
        };

        view = device->get_handle().createImageViewUnique(create_info);

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