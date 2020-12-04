#include "odin/odin.h"

#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void cube_draw(odin_render_device render_device, odin_draw_data draw_data, void* user_data);


typedef struct default_vertex
{

    float position[3];
    float color[4];

} default_vertex_t;


static default_vertex_t verts[4] =
{
    {{-0.5f, -0.5f, 0.0f},  {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f},   {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f},   {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f},    {1.0f, 1.0f, 1.0f}}
};

static uint32_t indices[4] =
{
    0, 1, 2, 3
};


typedef struct cube_example_data
{

    odin_render_pass        gbuffer;
    odin_framebuffer        gbuffer_framebuffer;
    odin_vertex_buffer      vertex_buffer;
    odin_index_buffer       index_buffer;
    odin_pipeline           pipeline;

} cube_example_data;

cube_example_data cube_data = { 0 };


int main()
{

    odin_api choosen_api = odin_api_vulkan;
    odin_load_api(choosen_api);

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

    odin_set_physical_device(render_device, physical_devices[0], window);
    

    /* Describe the vertex assembly to create vertex buffers. */
    odin_vertex_assembly vertex_assembly;
    odin_vertex_assembly_create(render_device, &vertex_assembly, 0, sizeof(default_vertex_t), 2);
    
    odin_vertex_assembly_describe_element(render_device, vertex_assembly, 0, odin_vertex_element_format_vec3f);
    odin_vertex_assembly_describe_element(render_device, vertex_assembly, 1, odin_vertex_element_format_vec4f);


    odin_vertex_buffer vertex_buffer;
    odin_vertex_buffer_create(render_device, &vertex_buffer, vertex_assembly, 4, verts);

    odin_index_buffer index_buffer;
    odin_index_buffer_create(render_device, &index_buffer, 4, indices);


    /* Load the image using stb_image. */
    int texture_width = 0, texture_height = 0, texture_channels = 0;
    stbi_uc* texture_data = stbi_load("default.png", &texture_width, &texture_height, &texture_channels, 4);

    assert(texture_data);

    odin_image texture;
    odin_image_create(render_device, &texture, odin_image_format_rgba_8_srgb, texture_width, texture_height, 1, 3, odin_image_samples_1x);

    odin_image_upload_data(render_device, texture, texture_data);


    /* Create the render passes. */
    odin_image color_frame;
    odin_image_create(render_device, &color_frame, odin_image_format_rgba_8_srgb, 1080, 720, 1, 1, odin_image_samples_1x);

    odin_render_pass_create(render_device, &cube_data.gbuffer, 1, &color_frame);

    odin_framebuffer_create(render_device, &cube_data.gbuffer_framebuffer, 1080, 720, cube_data.gbuffer, 1, &color_frame);


    
    odin_shader_code simple_shader_vert_code;
    odin_shader_code simple_shader_frag_code;

    switch (choosen_api)
    {
    /* For vulkan we must load spirv. */
    case odin_api_vulkan:
        odin_pipeline_shader_load_code(&simple_shader_vert_code, "shaders/vulkan/simple_shader_vert.spv");
        odin_pipeline_shader_load_code(&simple_shader_frag_code, "shaders/vulkan/simple_shader_frag.spv");
        break;
    
    default:
        break;
    }


    odin_shader simple_shader_vert;
    odin_shader simple_shader_frag;

    odin_pipeline_shader_create(render_device, &simple_shader_vert, odin_shader_stage_vertex, simple_shader_vert_code);
    odin_pipeline_shader_create(render_device, &simple_shader_frag, odin_shader_stage_fragment, simple_shader_frag_code);


    odin_pipeline simple_shader;
    odin_pipeline_create(render_device, &simple_shader, simple_shader_vert, simple_shader_frag, cube_data.gbuffer, 1, &vertex_assembly);


    /*
    odin_draw_prepare(render_device);


    while(true)
    {
        odin_draw_frame(render_device, cube_draw);
    }

    odin_draw_done(render_device);
    */


    /* Destroy resources. */
    odin_pipeline_destroy(render_device, simple_shader);

    odin_framebuffer_destroy(render_device, cube_data.gbuffer_framebuffer);

    odin_render_pass_destroy(render_device, cube_data.gbuffer);

    odin_image_destroy(render_device, color_frame);

    odin_image_destroy(render_device, texture);

    odin_index_buffer_destroy(render_device, index_buffer);
    odin_vertex_buffer_destroy(render_device, vertex_buffer);

    odin_vertex_assembly_destroy(render_device, vertex_assembly);


    odin_window_destroy(render_device, window);

    odin_terminate(render_device);
}


void cube_draw(odin_render_device render_device, odin_draw_data draw_data, void* user_data)
{

    cube_example_data* draw_cube_data = (cube_example_data*)user_data;

    odin_draw_command_begin_render_pass(render_device, draw_data, draw_cube_data->gbuffer, draw_cube_data->gbuffer_framebuffer);

        odin_draw_command_set_vertex_buffer(render_device, draw_data, draw_cube_data->vertex_buffer);
        odin_draw_command_set_index_buffer(render_device, draw_data, draw_cube_data->index_buffer);

        odin_draw_command_set_pipeline(render_device, draw_data, draw_cube_data->pipeline);

    odin_draw_command_end_render_pass(render_device, draw_data, draw_cube_data->gbuffer);

}

