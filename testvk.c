#include <stdio.h>
#include <stdlib.h>

#define VOLK_IMPLEMENTATION
#include "volk/volk.h"

static const char* VkResultToString(VkResult res) {
        switch (res) {
#define CASE(x) case VK_##x: return #x;
        CASE(SUCCESS)                       CASE(NOT_READY)
        CASE(TIMEOUT)                       CASE(EVENT_SET)
        CASE(EVENT_RESET)                   CASE(INCOMPLETE)
        CASE(ERROR_OUT_OF_HOST_MEMORY)      CASE(ERROR_OUT_OF_DEVICE_MEMORY)
        CASE(ERROR_INITIALIZATION_FAILED)   CASE(ERROR_DEVICE_LOST)
        CASE(ERROR_MEMORY_MAP_FAILED)       CASE(ERROR_LAYER_NOT_PRESENT)
        CASE(ERROR_EXTENSION_NOT_PRESENT)   CASE(ERROR_FEATURE_NOT_PRESENT)
        CASE(ERROR_INCOMPATIBLE_DRIVER)     CASE(ERROR_TOO_MANY_OBJECTS)
        CASE(ERROR_FORMAT_NOT_SUPPORTED)    CASE(ERROR_FRAGMENTED_POOL)
        CASE(ERROR_UNKNOWN)                 CASE(ERROR_OUT_OF_POOL_MEMORY)
        CASE(ERROR_INVALID_EXTERNAL_HANDLE) CASE(ERROR_FRAGMENTATION)
        CASE(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        CASE(PIPELINE_COMPILE_REQUIRED)      CASE(ERROR_SURFACE_LOST_KHR)
        CASE(ERROR_NATIVE_WINDOW_IN_USE_KHR) CASE(SUBOPTIMAL_KHR)
        CASE(ERROR_OUT_OF_DATE_KHR)          CASE(ERROR_INCOMPATIBLE_DISPLAY_KHR)
        CASE(ERROR_VALIDATION_FAILED_EXT)    CASE(ERROR_INVALID_SHADER_NV)
#ifdef VK_ENABLE_BETA_EXTENSIONS
        CASE(ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
        CASE(ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
        CASE(ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
        CASE(ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
        CASE(ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
        CASE(ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
#endif
        CASE(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        CASE(ERROR_NOT_PERMITTED_KHR)
        CASE(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        CASE(THREAD_IDLE_KHR)        CASE(THREAD_DONE_KHR)
        CASE(OPERATION_DEFERRED_KHR) CASE(OPERATION_NOT_DEFERRED_KHR)
        default: return "unknown";
        }
#undef CASE
}
#define VKTRY(call) do { \
        VkResult res = (call); \
        if (res != VK_SUCCESS) { \
            fprintf(stderr, "Failed $call: %s (%d)\n", \
                    VkResultToString(res), res); exit(1); \
        } \
    } while (0)

int main() {
    VKTRY(volkInitialize());

    VkInstance instance; {
        VkInstanceCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        const char* validationLayers[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        createInfo.enabledLayerCount = sizeof(validationLayers)/sizeof(validationLayers[0]);
        createInfo.ppEnabledLayerNames = validationLayers;

        const char* enabledExtensions[] = {
            // 2 extensions for non-X11/Wayland display
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_DISPLAY_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };
        createInfo.enabledExtensionCount = sizeof(enabledExtensions)/sizeof(enabledExtensions[0]);
        createInfo.ppEnabledExtensionNames = enabledExtensions;
        
        VkResult res = vkCreateInstance(&createInfo, NULL, &instance);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "Failed vkCreateInstance: %s (%d)\n",
                    VkResultToString(res), res);
            if (res == VK_ERROR_LAYER_NOT_PRESENT) {
                fprintf(stderr, "\nIt looks like a required layer is missing.\n"
                        "Did you install `vulkan-validationlayers`?\n");
            }
            exit(1);
        }
    }
    volkLoadInstance(instance);

    VkPhysicalDevice physicalDevice; {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
        if (physicalDeviceCount == 0) {
            fprintf(stderr, "Failed to find Vulkan physical device\n"); exit(1);
        }
        printf("Gpu: Found %d Vulkan devices\n", physicalDeviceCount);
        VkPhysicalDevice physicalDevices[physicalDeviceCount];
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

        physicalDevice = physicalDevices[0];
    }

    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t computeQueueFamilyIndex = UINT32_MAX; {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
        VkQueueFamilyProperties queueFamilies[queueFamilyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
        for (int i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                computeQueueFamilyIndex = i;
            }
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueFamilyIndex = i;
                break;
            }
        }
        if (graphicsQueueFamilyIndex == UINT32_MAX) {
            fprintf(stderr, "Failed to find a Vulkan graphics queue family\n"); exit(1);
        }
        if (computeQueueFamilyIndex == UINT32_MAX) {
            fprintf(stderr, "Failed to find a Vulkan compute queue family\n"); exit(1);
        }
    }

    VkDevice device; {
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {0};

        const char *deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = sizeof(deviceExtensions)/sizeof(deviceExtensions[0]);
        createInfo.ppEnabledExtensionNames = deviceExtensions;

        /* VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {0}; */
        /* descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES; */
        /* descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE; */
        /* // TODO: Do we need more descriptor indexing features? */
        /* createInfo.pNext = &descriptorIndexingFeatures; */

        VKTRY(vkCreateDevice(physicalDevice, &createInfo, NULL, &device));
    }

    uint32_t propertyCount;
    vkEnumerateInstanceLayerProperties(&propertyCount, NULL);
    VkLayerProperties layerProperties[propertyCount];
    vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties);

    VkSurfaceKHR surface; {
        uint32_t displayCount;
        vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, NULL);
        printf("Gpu: Found %d displays\n", displayCount);
        if (displayCount == 0) {
            fprintf(stderr, "Gpu: No displays found\n"); exit(1);
        }
    }
}
