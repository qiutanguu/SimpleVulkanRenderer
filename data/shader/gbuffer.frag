#version 450

layout (binding = 2) uniform sampler2D basecolor_tex;
layout (binding = 3) uniform sampler2D normal_tex;

layout (location = 0) in vec3 vary_normal;
layout (location = 1) in vec2 vary_uv0;
layout (location = 2) in vec3 vary_color;
layout (location = 3) in vec4 vary_tangent;
layout (location = 4) in vec3 vary_worldpos;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_basecolor;

void main() 
{
	out_position = vec4(vary_worldpos, 1.0);

	
	vec3 N = normalize(vary_normal);
	vec3 T = normalize(vary_tangent.xyz);
	vec3 B = normalize(cross(N, T) * vary_tangent.w);
	mat3 TBN = mat3(T, B, N);

	vec3 tnorm = TBN * normalize(texture(normal_tex, vary_uv0).xyz * 2.0 - vec3(1.0));
	out_normal = vec4(tnorm, 1.0);
	out_basecolor = texture(basecolor_tex, vary_uv0);
}