#include "mesh.hpp"
#include <vulkan/vulkan_enums.hpp>
#include "logging.hpp"
#include "memory.hpp"

namespace engine {

    

    Mesh::Mesh (vk::PhysicalDevice& physical_device, vk::Device& device, vk::SurfaceKHR& surface) : device(device) {

        vertices = {
        {{0.00f, -0.05f}, {1.0f, 0.0f, 0.0f}},
        {{0.05f, 0.05f}, {0.0f, 1.0f, 0.0f}},
        {{-0.05f, 0.05f}, {0.0f, 0.0f, 1.0f}}
        };

        auto size = sizeof(Vertex) * vertices.size();

        using enum vk::MemoryPropertyFlagBits; using enum vk::BufferUsageFlagBits;
        auto [staging_buffer, staging_memory] = create_buffer(physical_device, device, size, eTransferSrc, eHostVisible | eHostCoherent);

        void* mapped_memory = device.mapMemory(staging_memory, 0, size);
        std::memcpy(mapped_memory, vertices.data(), size);
        device.unmapMemory(staging_memory);

        auto [buffer, memory] = create_buffer(physical_device, device, size, eTransferDst | eVertexBuffer, eDeviceLocal);

        copy_buffer(physical_device, device, surface, staging_buffer, buffer, size);

        vertex_buffer = {buffer, memory};

        device.destroyBuffer(staging_buffer);
        device.freeMemory(staging_memory);

    }

    Mesh::~Mesh ( ) {

        device.destroyBuffer(vertex_buffer.buffer);
        device.freeMemory(vertex_buffer.memory);

    }

}