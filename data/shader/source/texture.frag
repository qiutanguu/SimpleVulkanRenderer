#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 2) uniform sampler2D base_color_texture;

layout(location = 0) in vec3 vary_color;
layout(location = 1) in vec2 vary_uv;

layout(location = 0) out vec4 out_color;

void main() 
{
    out_color = texture(base_color_texture, vary_uv);
}