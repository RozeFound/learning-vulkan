#pragma once

#include <vector>

#include "essentials.hpp"
#include "memory.hpp"
#include "device.hpp"
#include "shaders.hpp"


namespace engine {

    struct Mesh {

        std::vector<Vertex> vertices;
        std::unique_ptr<Buffer> vertex_buffer;

        Mesh (std::shared_ptr<Device> device);

    };

}