#version 450

layout (binding = 2) uniform sampler2D basecolor_tex;
layout (binding = 3) uniform sampler2D normal_tex;
layout (binding = 4) uniform sampler2D metalic_tex;
layout (binding = 5) uniform sampler2D roughness_tex;
layout (binding = 6) uniform sampler2D mask_tex;

layout (location = 0) in vec3 vary_normal;
layout (location = 1) in vec2 vary_uv0;
layout (location = 2) in vec3 vary_color;
layout (location = 3) in vec4 vary_tangent;
layout (location = 4) in vec3 vary_worldpos;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_basecolor;

vec3 get_normal_from_map()
{
    vec3 normal_tangent_space = normalize(texture(normal_tex, vary_uv0).xyz * 2.0 - vec3(1.0));

    vec3 Q1  = dFdx(vary_worldpos);
    vec3 Q2  = dFdy(vary_worldpos);
    vec2 st1 = dFdx(vary_uv0);
    vec2 st2 = dFdy(vary_uv0);

    vec3 N  = normalize(vary_normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * normal_tangent_space);
}

void main() 
{
    float mask = texture(mask_tex,vary_uv0).r;
    if(mask < 0.1f)
    {
        discard;
    }

    float roughness = texture(roughness_tex,vary_uv0).r;
    float metalic = texture(metalic_tex,vary_uv0).r;

	vec3 world_space_normal = get_normal_from_map();
	
	out_position = vec4(vary_worldpos, roughness);
	out_normal = vec4(normalize(vary_normal), metalic);
	out_basecolor = texture(basecolor_tex, vary_uv0);
}