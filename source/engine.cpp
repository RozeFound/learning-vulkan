#include <vector>

#include "instance.hpp"
#include "device.hpp"
#include "engine.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "logging.hpp"

namespace engine {

    Engine::Engine ( ) {

        LOG_INFO("Creating Engine instance...");

        make_window();
        make_instance();
        make_device();
        
    }

    Engine::~Engine ( ) {

        LOG_INFO("Destroying Engine...");

        if (debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

        pipeline.destroy();
        swapchain.destroy();
        device.destroy();

        instance.destroySurfaceKHR(surface);
        instance.destroy();

        glfwTerminate();

    }

    void Engine::make_window ( ) {

        LOG_INFO("Creating window...");

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

        if (window) {LOG_INFO("Successfully created {} window!", title);}
        else LOG_ERROR("Failed to create {} window", title);

    }

    void Engine::make_instance ( ) {


        if constexpr (debug) {

            auto version = vk::enumerateInstanceVersion();

            auto major = VK_API_VERSION_MAJOR(version);
            auto minor = VK_API_VERSION_MINOR(version);
            auto patch = VK_API_VERSION_PATCH(version);

            LOG_INFO("System support up to Vulkan {}.{}.{}", major, minor, patch);

        }

        instance = create_instance(title).value_or(nullptr);

        if constexpr (debug) {
            dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
            debug_messenger = make_debug_messenger(instance, dldi);
        }

        VkSurfaceKHR c_surface;

        if (glfwCreateWindowSurface(instance, window, nullptr, &c_surface) != VK_SUCCESS)
            LOG_ERROR("Cannot abstract GLFW surface for Vulkan");

        surface = c_surface;

    }

    void Engine::make_device ( ) {

        physical_device = get_physical_device(instance).value_or(nullptr);
        device = create_logical_device(physical_device, surface).value_or(nullptr);

        auto indices = get_queue_family_indices(physical_device, surface);
        graphics_queue = device.getQueue(indices.graphics_family.value(), 0);
        present_queue = device.getQueue(indices.present_family.value(), 0);

        swapchain = SwapChain(physical_device, device, surface, window);
        pipeline = PipeLine(device, swapchain);

    }

}