#include <memory>
#include <ranges>

#include <imgui.h>

#include "app.hpp"

#include "engine/utils/logging.hpp"

App::App (std::string_view title) {

    auto ec = glz::read_file(settings, "settings.json");

    window = create_window(settings.width, settings.height, title);

    graphics_engine = new engine::Engine(window);
    auto engine_settings = graphics_engine->get_settings();

    graphics_engine->ui_callback = [this, engine_settings] {
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

        static bool vsync = engine_settings.vsync;
        static int fps = engine_settings.fps_limit;
        
        ImGui::Begin("Preferences", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if(ImGui::Checkbox("Verical Synchronization", &vsync))
            graphics_engine->set<"vsync">(vsync);
        if(ImGui::InputInt("FPS Limit", &fps, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
            graphics_engine->set<"fps_limit">(fps);
        ImGui::End();

        ImGui::Begin("Available Objects", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        for(auto[key, value] : objects)
            if(ImGui::RadioButton(key.data(), settings.selected_object == key))
                settings.selected_object = key;
        ImGui::End();
    };

}

App::~App ( ) {

    auto ec = glz::write_file(settings, "settings.json");

    delete graphics_engine;
    glfwTerminate();

}

GLFWwindow* App::create_window (std::size_t width, std::size_t height, std::string_view title) {

    logi("Creating window...");

    glfwInit();

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

    glfwSetWindowSizeLimits(window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetWindowUserPointer(window, this);

    auto callback = [](GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

        app->graphics_engine->is_framebuffer_resized = true;

        if (!app->settings.fullscreen) {
            app->settings.height = height;
            app->settings.width = width;
        }

    }; glfwSetFramebufferSizeCallback(window, callback);

    if (window) logi("Successfully created {} window!", title);
    else loge("Failed to create {} window", title);

    return window;

}

void App::set_fullscreen (bool state) {

    if (state) {

        auto monitor = glfwGetPrimaryMonitor();

        int x, y, width, height; 
        glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

        glfwSetWindowMonitor(window, monitor, x, y, width, height, GLFW_DONT_CARE);

    } 
    else glfwSetWindowMonitor(window, nullptr, 0, 0, settings.width, settings.height, 0);

    settings.fullscreen = state;

}

void App::run ( ) {

    auto vertices = std::vector<engine::Vertex> {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    auto indices = std::vector<uint16_t> { 0, 1, 2, 2, 3, 0,
                                           4, 5, 6, 6, 7, 4 };


    objects["Rimuru Tempest"] = std::make_shared<engine::Object>("textures/image.jpg", vertices, indices);
    objects["Viking Room"] = std::make_shared<engine::Object>("textures/viking_room.png", "models/viking_room.obj");

    auto callback = [] (GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

        if (mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {

            if (key == GLFW_KEY_F) app->set_fullscreen(!app->settings.fullscreen);

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
        graphics_engine->draw(objects.at(settings.selected_object));

    }

}