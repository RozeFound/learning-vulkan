#pragma once

#include <map>
#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

struct Settings {

    bool fullscreen = false;
    std::size_t width = 800;
    std::size_t height = 600;

    std::string selected_object = "Viking Room";

    GLZ_LOCAL_META(Settings, width, height, selected_object);

};

class App {

    Settings settings;
    engine::Engine* graphics_engine;
    GLFWwindow* window;

    GLFWwindow* create_window (std::size_t width, std::size_t height, std::string_view title);
    void set_fullscreen (bool state);

    std::map<std::string_view, std::shared_ptr<engine::Object>> objects;

    public:

    App (std::string_view title);
    ~App ( );

    void run();

};