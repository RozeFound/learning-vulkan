#include "mesh.hpp"
#include "memory.hpp"

namespace engine {

    Mesh::Mesh (std::shared_ptr<Device> device) {

        vertices = {
        {{0.00f, -0.05f}, {1.0f, 0.0f, 0.0f}},
        {{0.05f, 0.05f}, {0.0f, 1.0f, 0.0f}},
        {{-0.05f, 0.05f}, {0.0f, 0.0f, 1.0f}}
        };

        auto size = sizeof(Vertex) * vertices.size();

        vertex_buffer = std::make_unique<Buffer>(device, size, vk::BufferUsageFlagBits::eVertexBuffer, true);
        vertex_buffer->write(vertices.data());

    }

}