#include "mesh.hpp"
#include <memory>
#include "memory.hpp"

namespace engine {

    Mesh::Mesh (std::shared_ptr<Device> device) {

        vertices = vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

        auto size = sizeof(Vertex) * vertices.size();

        vertex_buffer = std::make_unique<Buffer>(device, size, vk::BufferUsageFlagBits::eVertexBuffer, true);
        vertex_buffer->write(vertices.data());

        indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };

        index_buffer = std::make_unique<Buffer>(device, indices.size() * sizeof(uint16_t), vk::BufferUsageFlagBits::eIndexBuffer, true);
        index_buffer->write(indices.data());

    }

}