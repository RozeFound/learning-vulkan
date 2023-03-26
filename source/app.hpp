#pragma once

#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

class App {

    engine::Engine* graphics_engine;
    GLFWwindow* window;

    GLFWwindow* create_window (std::size_t width, std::size_t height, std::string_view title);

    std::shared_ptr<engine::Object> object;

    public:

    App (std::size_t width, std::size_t height, std::string_view title);
    ~App () { delete graphics_engine; glfwTerminate(); }

    void run();

};