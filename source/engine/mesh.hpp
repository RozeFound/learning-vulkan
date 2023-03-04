#pragma once

#include <vector>

#include "memory.hpp"
#include "device.hpp"
#include "primitives.hpp"


namespace engine {

    struct Mesh {

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        std::unique_ptr<Buffer> vertex_buffer;
        std::unique_ptr<Buffer> index_buffer;

        Mesh ( );

    };

}