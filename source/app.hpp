#pragma once

#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

class App {

    engine::Engine* graphics_engine;

    GLFWwindow* window;

    std::string title;

    uint32_t frames;

    double last_time, current_time;

    GLFWwindow* create_window (std::size_t width, std::size_t height, std::string_view title);

    void calculate_framerate ( );
    static void on_ui_update ( );

    public:

    App (std::size_t width, std::size_t height, std::string_view title);
    ~App () { delete graphics_engine; glfwTerminate(); }

    void run();

};