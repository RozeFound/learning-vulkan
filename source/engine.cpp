#include <vector>

#include "instance.hpp"
#include "device.hpp"
#include "engine.hpp"
#include "logging.hpp"

Engine::Engine ( ) {

    LOG_INFO("Creating Engine instance...");

    make_window();
    make_instance();
    make_device();
    make_swapchain();
    
}

Engine::~Engine ( ) {

    LOG_INFO("Destroying Engine...");

    if (logging::debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

    device.destroySwapchainKHR(swapchain);
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


    if constexpr (logging::debug) {

        auto version = vk::enumerateInstanceVersion();

        auto major = VK_API_VERSION_MAJOR(version);
        auto minor = VK_API_VERSION_MINOR(version);
        auto patch = VK_API_VERSION_PATCH(version);

        LOG_INFO("System support up to Vulkan {}.{}.{}", major, minor, patch);

    }

    instance = vk::instance::create_instance(title).value_or(nullptr);

    if constexpr (logging::debug) {
        dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
        debug_messenger = logging::make_debug_messenger(instance, dldi);
    }

}

void Engine::make_device ( ) {

    physical_device = vk::device::get_physical_device(instance).value_or(nullptr);

     VkSurfaceKHR c_surface;

    if (glfwCreateWindowSurface(instance, window, nullptr, &c_surface) != VK_SUCCESS)
        LOG_ERROR("Cannot abstract GLFW surface for Vulkan");

    surface = c_surface;

    device = vk::device::create_logical_device(physical_device, surface).value_or(nullptr);

    auto indices = vk::device::get_queue_family_indices(physical_device, surface);
    graphics_queue = device.getQueue(indices.graphics_family.value(), 0);
    present_queue = device.getQueue(indices.present_family.value(), 0);

}

void Engine::make_swapchain ( ) {

    auto capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
    auto formats = physical_device.getSurfaceFormatsKHR(surface);
    auto modes = physical_device.getSurfacePresentModesKHR(surface);

    auto present_mode = vk::PresentModeKHR::eFifo;

    for (auto& mode : modes) 
        if (mode == vk::PresentModeKHR::eMailbox) {
            present_mode = mode; break; }

    auto min_width = std::max(capabilities.minImageExtent.width, static_cast<uint32_t>(width));
    auto min_height = std::max(capabilities.minImageExtent.height, static_cast<uint32_t>(height));

    auto extent = vk::Extent2D { 
        .width = std::min(min_width, capabilities.maxImageExtent.width),
        .height = std::min(min_height, capabilities.maxImageExtent.height)
    };

    auto create_info = vk::SwapchainCreateInfoKHR {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = surface, 
        .minImageCount = capabilities.minImageCount,
        .imageFormat = formats.at(0).format,
        .imageColorSpace = formats.at(0).colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        // We'll set sharing mode and queue indices down //
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
    };

    auto indices = vk::device::get_queue_family_indices(physical_device, surface);
    uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

    if (indices.graphics_family != indices.present_family) {
			create_info.imageSharingMode = vk::SharingMode::eConcurrent;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
	} else create_info.imageSharingMode = vk::SharingMode::eExclusive;

    try {
        swapchain = device.createSwapchainKHR(create_info);
        LOG_INFO("Successfully created SwapChain")
    } catch (vk::SystemError err) {
        LOG_ERROR("Failed to create SwapChain");
    }

}