#version 450

layout (location = 0) in vec2 vary_uv0;
layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform sampler2D scenecolor_tex;

void main() 
{
	vec3 samplecolor = texture(scenecolor_tex, vary_uv0).xyz;
    out_color = vec4(samplecolor,1.0f);
}