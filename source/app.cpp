#include <memory>
#include <ranges>

#include <imgui.h>

#include "app.hpp"

#include "engine/utils/logging.hpp"

App::App (std::size_t width, std::size_t height, std::string_view title) {

    this->title = title;

    window = create_window(width, height, title);
    graphics_engine = new engine::Engine(window);
    graphics_engine->on_ui_update = on_ui_update;

}

GLFWwindow* App::create_window (std::size_t width, std::size_t height, std::string_view title) {

    logi("Creating window...");

    glfwInit();

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);

    auto callback = [](GLFWwindow* window, int width, int height) {

        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->graphics_engine->is_framebuffer_resized = true;

    };

    glfwSetFramebufferSizeCallback(window, callback);

    if (window) logi("Successfully created {} window!", title);
    else loge("Failed to create {} window", title);

    return window;

}

void App::on_ui_update ( ) {

    // ImGui::ShowDemoWindow();

    ImGuiIO& io = ImGui::GetIO();

    static float min_fps = 0, max_fps = 0;
    static auto frame_times = std::array<float, 100>();

    min_fps = std::min(min_fps, io.Framerate);
    max_fps = std::max(max_fps, io.Framerate);

    frame_times.back() = 1000.0f / io.Framerate;

    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Frametime %.3f/ms", frame_times.back());
    ImGui::PlotLines("", frame_times.data(), frame_times.size());
    ImGui::Text("FPS %.1f/s, min %.1f/s, max %.1f/s", io.Framerate, min_fps, max_fps);
    ImGui::End();

    std::ranges::rotate(frame_times, frame_times.begin() + 1);
        
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