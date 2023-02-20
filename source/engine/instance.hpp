#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    void log_supported_features ( );
    vk::Instance create_instance ( );

}