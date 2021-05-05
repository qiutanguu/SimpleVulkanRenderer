#ifndef RANDOM_GLSL
#define RANDOM_GLSL

float random(vec3 seed, int i)
{
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

vec2 vogel_disk_sample(int sample_index, int samples_count, float phi)
{
    float r = sqrt(sample_index + 0.5f) / sqrt(samples_count);
	float theta = sample_index * 2.4f + phi;

	float sine = sin(theta);
    float cosine = cos(theta);

	return vec2(r * cosine, r * sine);
}

float interleaved_gradient_noise(vec2 position_screen)
{
    vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
    return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

#endif