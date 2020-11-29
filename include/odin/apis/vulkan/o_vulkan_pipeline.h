#ifndef __ODIN_VULKAN_PIPELINE_H__
#define __ODIN_VULKAN_PIPELINE_H__


#include "odin/o_pipeline.h"
#include "odin/o_render_device.h"

#include <vulkan/vulkan.h>


/** \brief The vulkan interpretation of a shader. */
typedef struct odin_vulkan_shader_t
{

    VkShaderModule module;

} odin_vulkan_shader_t, *odin_vulkan_shader;

/** \brief The vulkan */
typedef struct odin_vulkan_pipeline_t
{

    VkPipeline pipeline;
    VkPipelineLayout layout;

} odin_vulkan_pipeline_t, *odin_vulkan_pipeline;


/** \brief Loads shader code on an API basis. */
void **odin_vulkan_pipeline_shader_load_code(const char* path);

/** \brief Creates a shader for a pipeline. */
void odin_vulkan_pipeline_shader_create(odin_render_device render_device, odin_shader *shader, odin_shader_stage stage, void **shader_code);

/** \brief Creates a pipeline from shaders.  */
void odin_vulkan_pipeline_create(odin_render_device render_device, odin_pipeline *pipeline, odin_shader vertex_shader, odin_shader fragment_shader, int vertex_assemblys_count, odin_vertex_assembly* vertex_assemblys);

/** \brief Destroys a pipeline from use. */
void odin_vulkan_pipeline_destroy(odin_render_device render_device, odin_pipeline pipeline);


#endif /* __ODIN_VULKAN_PIPELINE_H__ */