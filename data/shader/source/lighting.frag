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


// 法线分布函数
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// 几何自阴影函数
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// 菲涅尔函数
vec3 F_Schlick(float cosTheta, float metallic,vec3 basecolor)
{
	vec3 F0 = mix(vec3(0.04), basecolor, metallic);
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness,vec3 basecolor,vec3 lightcolor)
{
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0)
	{
		float rroughness = max(0.05, roughness);
		float D = D_GGX(dotNH, roughness); 
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		vec3 F = F_Schlick(dotNV, metallic,basecolor);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);
		color += spec * dotNL * lightcolor;
	}

	return color;
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
	vec3 fragcolor = dot(light_dir,world_normal) * ub_directional_light.color.xyz * basecolor.xyz;
	
    out_fragment_color = vec4(fragcolor, 1.0);	
}