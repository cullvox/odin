#include "odin/apis/vulkan/o_vulkan_api.h"

#include "odin/odin.h"
#include "odin/apis/vulkan/o_vulkan.h"


void odin_vulkan_api_load()
{

    odin_initialize = odin_vulkan_initialize;
    odin_terminate = odin_vulkan_terminate;

    odin_get_monitors = odin_vulkan_get_monitors;
    odin_window_create = odin_vulkan_window_create;
    odin_window_destroy = odin_vulkan_window_destroy;

}