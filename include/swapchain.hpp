#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "engine.hpp"
#include "logging.hpp"

namespace engine {

    std::optional<vk::SwapchainKHR> create_swapchain (engine::Engine& engine);

}