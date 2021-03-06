#include "odin/apis/vulkan/o_vulkan_platform.h"

#include <Windows.h>

#include "odin/o_log.h"
#include "odin/apis/vulkan/o_vulkan_window.h"
#include "../../../input/o_input_windows_internal.h"


const char **odin_vulkan_platform_get_validation_layers(uint32_t *validation_layers_count)
{

    static const uint32_t windows_validation_layers_count = 1;

    static const char *validation_layers[] = 
    { 
        "VK_LAYER_KHRONOS_validation" 
    };


    *validation_layers_count = windows_validation_layers_count;
    return validation_layers;

}

const char **odin_vulkan_platform_get_instance_extensions(uint32_t *instance_extensions_count)
{

    static const uint32_t windows_instance_extensions_count = 
    #if defined(NDEBUG)
        2;
    #else
        3;
    #endif

    static const char *windows_instance_extensions[] = 
    {
        "VK_KHR_surface", 
        "VK_KHR_win32_surface"
        #ifndef NDEBUG
            , VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        #endif  
    };


    *instance_extensions_count = windows_instance_extensions_count;
    return windows_instance_extensions;

}

const char **odin_vulkan_platform_get_device_extensions(uint32_t *device_extensions_count)
{

    static const uint32_t windows_device_extensions_count = 1;

    static const char *windows_device_extensions[] = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    *device_extensions_count = windows_device_extensions_count;
    return windows_device_extensions;

}


void odin_vulkan_platform_initialize(odin_vulkan_render_device render_device)
{

    /* Create the windows window class */
    WNDCLASSEX window_class = { 0 };
    window_class.cbSize = sizeof( WNDCLASSEX );
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = window_proc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = NULL;
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor( NULL, IDC_ARROW );
	window_class.hbrBackground = NULL;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "ODIN_VULKAN_WINDOW_CLASS";
	window_class.hIconSm = NULL;

    if(!RegisterClassEx(&window_class))
    {
        ODIN_ERROR("o_vulkan_platform_windows.c", "Could not create the window class!");
    }
}

void odin_vulkan_platform_terminate(odin_vulkan_render_device render_device)
{

}