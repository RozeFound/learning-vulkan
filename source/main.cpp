#include <memory>

#include "engine.hpp"

auto main (int argc, char** argv) -> int {

    auto engine = std::make_unique<Engine>();

    return 0;
}