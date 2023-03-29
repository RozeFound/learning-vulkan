#pragma once

#include <span>
#include <memory>
#include <string_view>

#include "memory.hpp"

#include "../utils/primitives.hpp"


namespace engine {

    class Model {

        using index_type = uint16_t;

        std::vector<Vertex> vertices;
        std::vector<index_type> indices;

        std::unique_ptr<Buffer> vertex_buffer;
        std::unique_ptr<Buffer> index_buffer;

        void update_buffers ( );

        public:

        Model (std::span<Vertex> vertices, std::span<index_type> indices);
        Model (std::string_view path);

        constexpr const vk::Buffer& get_vertex ( ) const { return vertex_buffer->get_handle(); }
        constexpr const vk::Buffer& get_index ( ) const { return index_buffer->get_handle(); }
        constexpr const std::size_t get_indices_count ( ) const { return indices.size(); }

    };

}