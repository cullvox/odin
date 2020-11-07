#include "odin/apis/vulkan/o_vulkan_window.h"

#include <Windows.h>
#include <dwmapi.h>

#include "aero/a_memory.h"

#include "odin/o_log.h"
#include "odin/apis/vulkan/o_vulkan_render_device.h"


static int temp_monitors_iterator = 0;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{

    int temp_monitors_count = GetSystemMetrics(SM_CMONITORS);

    if(temp_monitors_iterator > temp_monitors_count)
        temp_monitors_count = 0;
    
    odin_monitor* monitors = (odin_monitor *)dwData;

    monitors[temp_monitors_iterator] = malloc(sizeof(odin_monitor_t));

    monitors[temp_monitors_iterator]->width = lprcMonitor->right - lprcMonitor->left;
    monitors[temp_monitors_iterator]->height = lprcMonitor->bottom - lprcMonitor->top;

    temp_monitors_iterator++;

    return true;
}

/** \brief Gets a systems monitors and their information. */
void odin_vulkan_get_monitors(odin_render_device render_device, int *monitors_count, odin_monitor *monitors)
{

    /* First we need to get the display count. */
    int temp_monitors_count = GetSystemMetrics(SM_CMONITORS);
    *monitors_count = temp_monitors_count;

    if(monitors == NULL)
        return;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)monitors);        
}

typedef struct odin_windows_window_t
{

    odin_vulkan_window_t vulkan_window;
    HWND handle;

} odin_windows_window_t;

void odin_vulkan_window_create(odin_render_device render_device, odin_window* window, const char* title, int x, int y, int width, int height, odin_window_style style, bool fullscreen, odin_monitor monitor)
{

    /* Get the vulkan render device */
    odin_vulkan_render_device vulkan_render_device = (odin_vulkan_render_device)render_device;


    /* Create the windows window */
    DWORD window_style = 0;
    switch (style)
    {
    case odin_window_style_defalt:
        window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        break;
    
    case odin_window_style_minimal:
        window_style = 0;
        break;

    default:
        window_style = WS_OVERLAPPEDWINDOW;
        break;
    }

    /* Create the windows window. */
    odin_windows_window_t* vulkan_window = malloc(sizeof(odin_windows_window_t));
    aero_memset(vulkan_window, sizeof(odin_windows_window_t), 0);

    *window = (odin_window)vulkan_window;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND handle = CreateWindowEx(
        0,                              // Optional window styles.
        "ODIN_VULKAN_WINDOW_CLASS",                     // Window class
        title,    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );


    /* Make sure the style is correct. */
    switch (style)
    {
    
    case odin_window_style_minimal:
        SetWindowLong(handle, GWL_STYLE, 0); //remove all window styles, check MSDN for details
        break;

    default:
        break;
    }


    ShowWindow(handle, SW_SHOW);


    /* If the device has been created we can create the swapchain. */
    if(vulkan_render_device->device)
        odin_vulkan_window_swapchain_create(vulkan_render_device, vulkan_window);

}

void odin_vulkan_window_destroy(odin_render_device render_device, odin_window window)
{

    /* Get the vulkan render device. */

    /* Get the vulkan window. */
    odin_windows_window_t* vulkan_window = (odin_windows_window_t*)window;

    /* Destroy the swapchain. */
    odin_vulkan_window_swapchain_destroy()

    DestroyWindow(vulkan_window->handle);

}


void odin_vulkan_window_swapchain_create(odin_vulkan_render_device render_device, odin_vulkan_window window)
{

    /* Get the windows window. */
    odin_windows_window_t* windows_window = (odin_windows_window_t*)window;


    /* Get the surface capabilities */
    VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(render_device->physical_device, window->surface, &surface_capabilities);

    /* Get the min image count. */
    window->image_count = surface_capabilities.minImageCount + 1;
    if (window->image_count > surface_capabilities.maxImageCount)
    {
        window->image_count = surface_capabilities.maxImageCount;
    }
    
    /* Get the image format. */
    uint32_t surface_formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(render_device->physical_device, window->surface, &surface_formats_count, NULL);

    VkSurfaceFormatKHR* surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * surface_formats_count);
    aero_memset(surface_formats, sizeof(VkSurfaceFormatKHR) * surface_formats_count, 0);

    vkGetPhysicalDeviceSurfaceFormatsKHR(render_device->physical_device, window->surface, &surface_formats_count, surface_formats);

    bool found_surface_format = false;
    for(int i = 0; i < surface_formats_count; i++)
    {
        if(surface_formats[i].format == VK_FORMAT_R8G8B8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            found_surface_format = true;
            window->surface_format = surface_formats[i];
            break;
        }
    }

    if(!found_surface_format)
    {
        window->surface_format = surface_formats[0];
    }

    /* Get the image extent. */
    RECT frame;
    DwmGetWindowAttribute(windows_window->handle, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(RECT));

    //rect should be `0, 0, 1280, 1024`
    //frame should be `7, 0, 1273, 1017`
    
    window->surface_extent = (VkExtent2D){
        (uint32_t) frame.right - frame.left,
        (uint32_t) frame.bottom - frame.top
    };

    /* Get the queue family indices. */
    uint32_t queue_family_indices[] = {render_device->graphics_queue_index, render_device->present_queue_index};

    /* Get the present mode. */
    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(render_device->physical_device, window->surface, &present_modes_count, NULL);

    VkPresentModeKHR* present_modes = malloc(sizeof(VkPresentModeKHR) * present_modes_count);
    aero_memset(present_modes, sizeof(VkPresentModeKHR) * present_modes_count, 0);

    vkGetPhysicalDeviceSurfacePresentModesKHR(render_device->physical_device, window->surface, &present_modes_count, present_modes);

    bool found_present_mode = false;

    for(int i = 0; i < present_modes_count; i++)
    {
        /* Try to use mailbox because it provides the best configuration. */
        if(present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            window->present_mode = present_modes[i];
            found_present_mode = true;
            break;
        }
    }

    /* If no present mode is found FIFO or VSync can be used. */
    if(!found_present_mode)
        window->present_mode = VK_PRESENT_MODE_FIFO_KHR; 


    /* Create the swapchain. */
    VkSwapchainCreateInfoKHR swapchain_create_info = { 0 };
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = NULL;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = window->surface;
    swapchain_create_info.minImageCount = window->image_count;
    swapchain_create_info.imageFormat = window->surface_format.format;
    swapchain_create_info.imageColorSpace = window->surface_format.colorSpace;
    swapchain_create_info.imageExtent = window->surface_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = render_device->graphics_queue_index == render_device->present_queue_index ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = render_device->graphics_queue_index == render_device->present_queue_index ? 0 : 2;
    swapchain_create_info.pQueueFamilyIndices = render_device->graphics_queue_index == render_device->graphics_queue_index ? NULL : queue_family_indices;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = window->present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = NULL;
    
    if(vkCreateSwapchainKHR(render_device->device, &swapchain_create_info, NULL, &window->swapchain) != VK_SUCCESS)
        ODIN_ERROR("o_vulkan_window_windows.c", "Could not create a window's swapchain!");

}

void odin_vulkan_window_swapchain_destroy(odin_vulkan_render_device render_device, odin_vulkan_window window)
{

    vkDestroySwapchainKHR(render_device->device, window->swapchain, NULL);

}