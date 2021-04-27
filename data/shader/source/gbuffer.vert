#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv0;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec4 in_tangent;

layout(binding = 0) uniform uniform_buffer_view_project 
{
    mat4 view;
    mat4 proj;
} ub_vp;

layout(binding = 1) uniform uniform_buffer_model
{
    mat4 model;
} ub_m;

layout (location = 0) out vec3 vary_normal;
layout (location = 1) out vec2 vary_uv0;
layout (location = 2) out vec3 vary_color;
layout (location = 3) out vec4 vary_tangent;
layout (location = 4) out vec3 vary_worldpos;

void main() 
{
    gl_Position = ub_vp.proj * ub_vp.view * ub_m.model * vec4(in_pos, 1.0);
    mat3 normal_mat = transpose(inverse(mat3(ub_m.model)));

    vary_uv0 = in_uv0;
    vary_color = in_color;
    vary_worldpos = vec3(ub_m.model * vec4(in_pos, 1.0));
    vary_normal  = normal_mat * normalize(in_normal);	
    vary_tangent.xyz = normal_mat * normalize(in_tangent.xyz);
    vary_tangent.w = in_tangent.w;
}