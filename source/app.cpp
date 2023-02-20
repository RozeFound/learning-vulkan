#include <memory>

#include "app.hpp"
#include "engine.hpp"
#include "logging.hpp"

App::App (std::size_t width, std::size_t height, std::string_view title) {

    this->title = title;

    window = create_window(width, height, title);
    graphics_engine = std::make_unique<engine::Engine>(window);

}

GLFWwindow* App::create_window (std::size_t width, std::size_t height, std::string_view title) {

    LOG_INFO("Creating window...");

    glfwInit();

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

    if (window) {LOG_INFO("Successfully created {} window!", title);}
    else LOG_ERROR("Failed to create {} window", title);

    return window;

}

void App::run ( ) {

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        graphics_engine->draw();
        calculate_framerate();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

    }

}

void App::calculate_framerate ( ) {

    current_time = glfwGetTime();
    auto delta = current_time - last_time;

    if (delta >= 1) {

        uint64_t framerate = std::max(1.0, frames / delta);

        auto title = fmt::format("{} - {} FPS", this->title, framerate);
        glfwSetWindowTitle(window, title.c_str());

        last_time = current_time;
        frames = 0;

    }
    
    frames++;

}