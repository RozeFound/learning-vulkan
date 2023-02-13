#include <cstddef>
#include <string_view>

#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

class Engine {

    constexpr static bool debug = true;

    vk::DebugUtilsMessengerEXT debug_messenger;
    vk::DispatchLoaderDynamic dldi;

    std::size_t width = 800, height = 600;
    std::string_view title = "Learning Vulkan";

    GLFWwindow* window;
    vk::Instance instance;

    vk::PhysicalDevice physical_device;
    vk::Device device;
    vk::Queue graphics_queue;

    void create_window ( );
    void create_instance ( );
    void create_device ( );

public:

    Engine();
    ~Engine();

};