#pragma once

#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

struct Settings {

    bool fullscreen = false;
    std::size_t window_width = 800;
    std::size_t window_height = 600;

    GLZ_LOCAL_META(Settings, fullscreen, window_width, window_height);

};

class App {

    Settings settings;
    engine::Engine* graphics_engine;
    GLFWwindow* window;

    GLFWwindow* create_window (std::size_t width, std::size_t height, std::string_view title);
    void set_fullscreen (bool state);

    std::shared_ptr<engine::Object> object;

    public:

    App (std::size_t width, std::size_t height, std::string_view title);
    ~App ( );

    void run();

};