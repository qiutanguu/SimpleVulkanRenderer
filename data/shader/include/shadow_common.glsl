#ifndef SHADOW_COMMON_GLSL
#define SHADOW_COMMON_GLSL

#include "random.glsl"
#include "poisson_disk.glsl"
#include "math.glsl"

// 64次possion采样的pcf
float hardware_shadow_pcf(sampler2DShadow shadow_tex,vec4 shadow_coord,vec2 texel_size,float dilation)
{
	float shadowMaskColor = 0.0f;

	for (int x = 0; x < 64; x++)
	{
		vec2 offset_uv = texel_size * poisson_disk_64[x]  * dilation;
		float shadowMapCmp = texture(shadow_tex, vec3(shadow_coord.xy + offset_uv,shadow_coord.z));
		shadowMaskColor += shadowMapCmp;
	}
	shadowMaskColor /= 64.0f;

	return shadowMaskColor;
}

// 9次采样的pcf
float hardware_shadow_pcf_microsoft(sampler2DShadow shadow_tex,vec4 shadow_coord,vec2 texel_size,float dilation)
{
	float d1 = dilation * texel_size.x * 0.125;
    float d2 = dilation * texel_size.x * 0.875;
    float d3 = dilation * texel_size.x * 0.625;
    float d4 = dilation * texel_size.x * 0.375;

	shadow_coord.z -= 0.001f;
    float result = (
        2.0 * texture(shadow_tex,vec3(shadow_coord.xy,shadow_coord.z)) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2(-d2,  d1), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2(-d1, -d2), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2( d2, -d1), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2( d1,  d2), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2(-d4,  d3), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2(-d3, -d4), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2( d4, -d3), shadow_coord.z )) +
        texture( shadow_tex, vec3(shadow_coord.xy + vec2( d3,  d4), shadow_coord.z ))
    ) / 10.0f;

	return result*result;
}

// pcss 查找遮挡物
vec2 pcss_search_blocker(vec4 shadow_coord,sampler2D shadowdepth_tex,vec2 texel_size,float dilation) 
{
	float blocker_depth = 0.0;
	float count = 0.0;
	for (int x = 0; x < 32; x++)
	{
		int index = int( 32.0 * random(shadow_coord.xyy,x) ) % 32;
		vec2 sample_uv = dilation * poisson_disk_32[index] * texel_size + shadow_coord.st;
		float dist = texture(shadowdepth_tex, sample_uv).r;

		if(shadow_coord.w > 0.0f && dist < shadow_coord.z)
		{
			blocker_depth += dist;
			count += 1.0f;
		}
	}

	if(count > 0.5f)
	{
		return vec2(blocker_depth / count, 1.0f);
	}
	else
	{
		return vec2(1.0f,0.0f);
	}
}

// pcss软阴影
float shadow_pcss(vec4 shadow_coord,sampler2DShadow shadowdepth_tex,sampler2D shadowdepth_normalsample,float pcf_dilation,float pcss_dilation,vec2 texel_size)
{
	vec2 ret = pcss_search_blocker(shadow_coord,shadowdepth_normalsample,texel_size, pcss_dilation);
	float avg_blocker_depth = ret.x;
	if(ret.y < 0.5f)
	{
		// 提前退出节省非阴影区域的pcf消耗。
		return 1.0f; 
	}

	float penumbra_size = max(shadow_coord.z - avg_blocker_depth,0.0f) / avg_blocker_depth * pcf_dilation;
	return hardware_shadow_pcf(shadowdepth_tex,shadow_coord,texel_size,penumbra_size);
}


#endif