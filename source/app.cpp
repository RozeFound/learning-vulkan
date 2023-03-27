#include <memory>
#include <ranges>

#include <imgui.h>

#include "app.hpp"

#include "engine/utils/logging.hpp"

App::App (std::size_t width, std::size_t height, std::string_view title) {

    window = create_window(width, height, title);
    graphics_engine = new engine::Engine(window);
    auto settings = graphics_engine->get_settings();

    graphics_engine->ui_callback = [this, settings] {
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

        static bool vsync = settings.vsync;
        static int fps = settings.fps_limit;
        
        ImGui::Begin("Preferences", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if(ImGui::Checkbox("Verical Synchronization", &vsync))
            graphics_engine->set<"vsync">(vsync);
        if(ImGui::InputInt("FPS Limit", &fps, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
            graphics_engine->set<"fps_limit">(fps);
        ImGui::End();
    };

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

    }; glfwSetFramebufferSizeCallback(window, callback);

    if (window) logi("Successfully created {} window!", title);
    else loge("Failed to create {} window", title);

    return window;

}

void App::run ( ) {

    auto viking_room = new engine::Object {
        .texture = engine::Texture("textures/viking_room.png"),
        .model = engine::Model("models/viking_room.obj")
    };

    object = std::shared_ptr<engine::Object>(viking_room);

    auto callback = [] (GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

        if (mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {

            if (key == GLFW_KEY_I) {
                auto state = app->graphics_engine->get_settings().gui_visible;
                app->graphics_engine->set<"gui_visible">(!state);
            }
            if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);

        }

        app->graphics_engine->ui_key_callback(window, key, scancode, action, mods);

    }; glfwSetKeyCallback(window, callback);

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        graphics_engine->draw(object);

    }

}