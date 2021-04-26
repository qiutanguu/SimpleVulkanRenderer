#version 450

layout (location = 0) in vec2 vary_uv;
layout (location = 0) out vec4 out_fragment_color;

layout(binding = 0) uniform uniform_directional_light 
{
    vec4 direction;
    vec4 color;
} ub_directional_light;

layout (binding = 1) uniform sampler2D gbuffer_position_worldspace;
layout (binding = 2) uniform sampler2D gbuffer_normal_worldspace;
layout (binding = 3) uniform sampler2D gbuffer_basecolor;

void main() 
{
	vec3 world_pos = texture(gbuffer_position_worldspace, vary_uv).rgb;
	vec3 world_normal = texture(gbuffer_normal_worldspace, vary_uv).rgb;
	vec4 basecolor  = texture(gbuffer_basecolor, vary_uv);
	
	vec3 light_dir = normalize(ub_directional_light.direction.xyz);
	vec3 fragcolor = dot(light_dir,world_normal) * ub_directional_light.color.xyz * basecolor.xyz;
	
    out_fragment_color = vec4(fragcolor, 1.0);	
}