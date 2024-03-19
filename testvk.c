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
}
