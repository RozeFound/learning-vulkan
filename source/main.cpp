#include <memory>
#include <vector>

#include "app.hpp"

auto main (const int argc, const char* const* argv) -> int {

    auto args = std::vector<std::string_view>(argv, argv + argc);
    auto program = args.at(0).substr(args.at(0).find_last_of("/") + 1);

    auto app = std::make_unique<App>(800, 600, program);

    app->run();

    return 0;
}