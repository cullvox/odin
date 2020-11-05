#include "odin/odin.h"

#include <stdlib.h>
#include <string.h>

int main()
{

    odin_load_api(odin_api_vulkan);

    odin_initialize_info initialize_info = { 0 };
    initialize_info.application_name = "Cube Example";
    initialize_info.application_version = (odin_version){ 0, 0, 1 };
    initialize_info.engine_name = "Example Engine";
    initialize_info.engine_version = (odin_version){ 0, 0, 1 };

    odin_render_device render_device;
    odin_initialize(&render_device, &initialize_info);

    /* Get the monitors count. */
    int monitors_count = 0;
    odin_get_monitors(render_device, &monitors_count, NULL);

    /* Create the monitors array. */
    odin_monitor* monitors = malloc(sizeof(odin_monitor) * monitors_count);
    memset(monitors, 0, sizeof(odin_monitor) * monitors_count);

    /* Get the monitors. */
    odin_get_monitors(render_device, &monitors_count, monitors);

    int width = monitors[0]->width;
    int height = monitors[0]->height;

    /* Create the window. */
    odin_window window = NULL;
    odin_window_create(render_device, &window, "Cube Example", 0, 0, monitors[0]->width / 2, monitors[0]->height / 2, odin_window_style_minimal, false, NULL);

    free(monitors);

    
    /* Get the physical devices. */
    int physical_devices_count = 0;
    odin_get_physical_devices(render_device, &physical_devices_count, NULL);

    odin_physical_device* physical_devices = malloc(sizeof(odin_physical_device) * physical_devices_count);
    memset(physical_devices, 0, sizeof(odin_physical_device) * physical_devices_count);

    odin_get_physical_devices(render_device, &physical_devices_count, physical_devices);


    odin_set_physical_device(render_device, physical_devices[0]);


    odin_window_destroy(render_device, window);

    odin_terminate(render_device);
}
