#include <ranges>
#include <set>

#include "device.hpp"

#include "../utils/logging.hpp"
#include "../utils/utils.hpp"

namespace engine {

    static std::weak_ptr<Device> device_instance;

    void Device::set_static_instance (std::shared_ptr<Device>& device) {

        device_instance = device;

    }

    const std::shared_ptr<Device> Device::get ( ) { 
        if (!device_instance.expired()) return device_instance.lock();
        else throw std::runtime_error("Device Instance has been expired!");
    }

    Device::Device (GLFWwindow* window) : window(window) {

        make_instance();

        auto result = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));
        if (result != VK_SUCCESS) loge("Cannot abstract GLFW surface for Vulkan");

        choose_physical_device();
        create_handle();

        auto create_info = VmaAllocatorCreateInfo {
            .physicalDevice = gpu,
            .device = handle,
            .instance = instance
        };

        vmaCreateAllocator(&create_info, &allocator);
        
    }

    Device::~Device ( ) {

        logi("Destroying Allocator");
        vmaDestroyAllocator(allocator);
        logi("Destroying Device");
        handle.destroy();
        logi("Destroying Surface");
        instance.destroySurfaceKHR(surface);
        logi("Destroying Instance");
        instance.destroy();

    }

    void Device::make_instance ( ) {

        uint32_t glfw_extension_count = 0;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        if (!glfw_extensions) logw("Failed to fetch required GLFW Extensions!");

        auto extensions = std::vector(glfw_extensions, glfw_extensions + glfw_extension_count);
        auto layers = std::vector<const char*>();

        if constexpr (debug) {

            extensions.push_back("VK_EXT_debug_utils");

            logv("Extensions to be requested: ");
            for (auto& extension : extensions) {
                logv("\t{}", extension);
            }

            layers.push_back("VK_LAYER_KHRONOS_validation");

            log_instance_properties();

        }

        auto enables = vk::ValidationFeatureEnableEXT { vk::ValidationFeatureEnableEXT::eBestPractices };
        auto features = vk::ValidationFeaturesEXT { 
             .enabledValidationFeatureCount = 1,
            .pEnabledValidationFeatures = &enables
        };

        auto app_info = vk::ApplicationInfo {
            .apiVersion = VK_VERSION_1_3
        };

        auto create_info = vk::InstanceCreateInfo {
            .pNext = &features,
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &app_info,
            .enabledLayerCount = to_u32(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = to_u32(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        try {
            instance = vk::createInstance(create_info);
            logi("Successfully created Instance");
            
        } catch(vk::SystemError err) {
            loge("Failed to create Instance");
        }

    } 

    void Device::choose_physical_device ( ) {

        auto suitable = [](vk::PhysicalDevice& device){

            for (auto& extension_properies : device.enumerateDeviceExtensionProperties()) {

                if (std::strcmp(extension_properies.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                    continue;
                
                return true;
            }  
            return false;

        };

        auto devices = instance.enumeratePhysicalDevices();

        auto device = std::ranges::find_if(devices, suitable);
        if (device != std::end(devices)) gpu = *device;
        else loge("Failed to get Physical Device");

        log_device_properties(gpu);

    }

    void Device::create_handle ( ) {

        auto indices = get_queue_family_indices(gpu, surface);

        auto queue_info = std::vector<vk::DeviceQueueCreateInfo>();
        auto queue_piority = 1.f;

        auto unique_indices = std::set {
            indices.transfer_family.value(),
            indices.graphics_family.value(), 
            indices.present_family.value()
        };

        for (const auto& queue_family : unique_indices) {

            auto create_info = vk::DeviceQueueCreateInfo {
                .flags = vk::DeviceQueueCreateFlags(),
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_piority
            }; queue_info.push_back(create_info);

        }

        auto device_features = vk::PhysicalDeviceFeatures();
        device_features.samplerAnisotropy = VK_TRUE;
        device_features.sampleRateShading = VK_TRUE;

        auto extensions = std::vector { "VK_KHR_swapchain" };
        auto layers = std::vector<const char*>();
        
        if constexpr (debug) layers.push_back("VK_LAYER_KHRONOS_validation");

        auto device_info = vk::DeviceCreateInfo {
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = to_u32(queue_info.size()),
            .pQueueCreateInfos = queue_info.data(),
            .enabledLayerCount = to_u32(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = to_u32(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = &device_features
        };

        try {
            handle = gpu.createDevice(device_info);
            logi("Device was successfully abstracted");
        } catch (vk::SystemError err) {
            loge("Failed to abstract Physical Device");
        }

    }
    
}