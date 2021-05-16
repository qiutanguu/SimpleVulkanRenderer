#version 450

layout (binding = 2) uniform sampler2D basecolor_tex;

layout (location = 0) in vec3 vary_normal;
layout (location = 1) in vec2 vary_uv0;
layout (location = 2) in vec3 vary_worldpos;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_basecolor;

void main() 
{
    float roughness = 0.3f;
    float metalic = 0.0f;

	vec3 world_space_normal = normalize(vary_normal);
	
	out_position = vec4(vary_worldpos, roughness);
	out_normal = vec4(world_space_normal, metalic);

    float g_buffer_id = 1.0f; // 角色shading model
	out_basecolor = vec4(texture(basecolor_tex, vary_uv0).xyz,g_buffer_id);
}