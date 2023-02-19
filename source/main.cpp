#include <memory>

#include "app.hpp"

auto main (int argc, char** argv) -> int {

    auto app = std::make_unique<App>(800, 600, "Learning Vulkan");

    app->run();

    return 0;
}