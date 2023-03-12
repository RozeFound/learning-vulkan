#include <unordered_map>

#include <tiny_obj_loader.h>

#include "model.hpp"

#include "../utils/logging.hpp"

namespace engine {

    Model::Model (std::string_view path) {

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string error;

        auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, &error, path.data());
        if (!result) log(error, log_level::error);

        auto unique_vertices = std::unordered_map<Vertex, index_type>();

        for (const auto& shape : shapes) for (const auto& index : shape.mesh.indices) {

            auto vertex = Vertex {
                .position = {
                    attrib.vertices.at(3 * index.vertex_index + 0),
                    attrib.vertices.at(3 * index.vertex_index + 1),
                    attrib.vertices.at(3 * index.vertex_index + 2),
                },
                .color = {1.f, 1.f, 1.f},
                .texture_coordinates = {
                    attrib.texcoords.at(2 * index.texcoord_index + 0),
                    1.f - attrib.texcoords.at(2 * index.texcoord_index + 1)
                }
            };

            if (!unique_vertices.contains(vertex)) {
                unique_vertices[vertex] = vertices.size();
                vertices.push_back(vertex);
            } 
            
            indices.push_back(unique_vertices.at(vertex));

        }

        vertex_buffer = std::make_unique<Buffer>(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer, false, true);
        vertex_buffer->write(vertices.data());

        index_buffer = std::make_unique<Buffer>(indices.size() * sizeof(index_type), vk::BufferUsageFlagBits::eIndexBuffer, false, true);
        index_buffer->write(indices.data());

    }

}