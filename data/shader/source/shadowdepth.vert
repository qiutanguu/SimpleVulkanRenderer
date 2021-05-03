#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv0;

layout(binding = 0) uniform uniform_buffer_view_project 
{
    mat4 view;
    mat4 proj;
} ub_vp;

layout(binding = 1) uniform uniform_buffer_model
{
    mat4 model;
} ub_m;

layout (location = 0) out vec2 vary_uv0;

void main()
{
	gl_Position = ub_vp.proj * ub_vp.view * ub_m.model * vec4(in_pos, 1.0);
    vary_uv0 = in_uv0;
}