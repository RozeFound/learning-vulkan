#include "mesh.hpp"
#include <vulkan/vulkan_enums.hpp>
#include "logging.hpp"
#include "memory.hpp"

namespace engine {

    

    Mesh::Mesh (Device& device) : device(device) {

        vertices = {
        {{0.00f, -0.05f}, {1.0f, 0.0f, 0.0f}},
        {{0.05f, 0.05f}, {0.0f, 1.0f, 0.0f}},
        {{-0.05f, 0.05f}, {0.0f, 0.0f, 1.0f}}
        };

        auto size = sizeof(Vertex) * vertices.size();

        using enum vk::MemoryPropertyFlagBits; using enum vk::BufferUsageFlagBits;
        auto [staging_buffer, staging_memory] = create_buffer(device, size, eTransferSrc, eHostVisible | eHostCoherent);

        void* mapped_memory = device.get_handle().mapMemory(staging_memory, 0, size);
        std::memcpy(mapped_memory, vertices.data(), size);
        device.get_handle().unmapMemory(staging_memory);

        auto [buffer, memory] = create_buffer(device, size, eTransferDst | eVertexBuffer, eDeviceLocal);

        copy_buffer(device, staging_buffer, buffer, size);

        vertex_buffer = {buffer, memory};

        device.get_handle().destroyBuffer(staging_buffer);
        device.get_handle().freeMemory(staging_memory);

    }

    Mesh::~Mesh ( ) {

        device.get_handle().destroyBuffer(vertex_buffer.buffer);
        device.get_handle().freeMemory(vertex_buffer.memory);

    }

}