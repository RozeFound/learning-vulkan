#pragma once

#include <cstddef>
#include <string_view>
#include <memory>

#include <GLFW/glfw3.h>

#include "engine.hpp"

class App {

    std::unique_ptr<engine::Engine> graphics_engine;

    GLFWwindow* window;

    GLFWwindow* create_window (std::size_t width, std::size_t height, std::string_view title);

    public:

    App (std::size_t width, std::size_t height, std::string_view title);
    ~App () { delete graphics_engine.release(); glfwTerminate(); };

    void run();

};