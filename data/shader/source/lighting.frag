#version 450

layout (location = 0) in vec2 vary_uv;
layout (location = 0) out vec4 out_fragment_color;

layout(binding = 0) uniform uniform_directional_light 
{
    vec4 direction;
    vec4 color;
	vec4 shadow_mix;
} ub_directional_light;

layout (binding = 1) uniform sampler2D gbuffer_position_worldspace;
layout (binding = 2) uniform sampler2D gbuffer_normal_worldspace;
layout (binding = 3) uniform sampler2D gbuffer_basecolor;

// 直射光的视图投影矩阵
layout(binding = 4) uniform uniform_directional_light_view_project 
{
    mat4 view;
    mat4 proj;
} ub_directional_light_vp;

layout (binding = 5) uniform sampler2DShadow directional_light_shadowdepth;
layout (binding = 6) uniform sampler2D directional_light_shadowdepth_tex;

#include "../include/shadow_common.glsl"

vec3 compute_shadow(vec3 fragworldpos)
{
	ivec2 tex_dim = textureSize(directional_light_shadowdepth, 0).xy;
	float dx = 1.0 / float(tex_dim.x);
	float dy = 1.0 / float(tex_dim.y);
	vec2 texel_size = vec2(dx, dy);
	vec4 shadow_clip_pos = ub_directional_light_vp.proj * ub_directional_light_vp.view * vec4(fragworldpos, 1.0);
	vec4 shadow_coord = shadow_clip_pos / shadow_clip_pos.w;
	
	if (shadow_coord.z <= -1.0 || shadow_coord.z >= 1.0)
	{
		return vec3(1.0f);
	}

	shadow_coord.z -= 0.001f;
	shadow_coord.st = shadow_coord.st * 0.5f + 0.5f;

	float pcss_dilation = ub_directional_light.shadow_mix.x;
	float pcf_dilation = ub_directional_light.shadow_mix.y;
	float shadow_swith = ub_directional_light.shadow_mix.z;

	float shadow_factor;
	if(shadow_swith < 0.0f) // pcss
	{
		shadow_factor = shadow_pcss(shadow_coord,directional_light_shadowdepth,directional_light_shadowdepth_tex,pcf_dilation,pcss_dilation,texel_size);
	}
	else // Microsoft pcf
	{
		shadow_factor = hardware_shadow_pcf_microsoft(directional_light_shadowdepth,shadow_coord,texel_size,2.0f);
	}

	 
	return vec3(shadow_factor);
} 

void main() 
{
	vec4 sampler_res0 = texture(gbuffer_position_worldspace, vary_uv);
	vec4 sampler_res1 = texture(gbuffer_normal_worldspace,vary_uv);

	vec3 world_pos = sampler_res0.rgb;
	vec3 world_normal = sampler_res1.rgb;
	
	float roughness = sampler_res0.a;
	float metalic = sampler_res1.a;

	vec4 basecolor  = texture(gbuffer_basecolor, vary_uv);
	
	vec3 light_dir = normalize(ub_directional_light.direction.xyz);
	float NoL = dot(light_dir,world_normal);
	vec3 fragcolor = max(0.0f,NoL) * ub_directional_light.color.xyz * basecolor.xyz;

	
	
	fragcolor = fragcolor * compute_shadow(world_pos) + vec3(0.05f) * basecolor.xyz;
	// fragcolor = fragcolor * hardware_shadow_pcf(directional_light_shadowdepth,NoL,world_pos,vary_uv,ub_directional_light.shadow_mix.x) + vec3(0.05f) * basecolor.xyz;
    // fragcolor = fragcolor * hardware_shadow_pcf_microsoft(directional_light_shadowdepth,NoL,world_pos,vary_uv) + vec3(0.05f) * basecolor.xyz;
	
	out_fragment_color = vec4(fragcolor, 1.0);	
}