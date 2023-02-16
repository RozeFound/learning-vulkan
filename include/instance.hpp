#pragma once

#include <string_view>
#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    void log_supported_features ( );
    std::optional<vk::Instance> create_instance (std::string_view application_name);

}