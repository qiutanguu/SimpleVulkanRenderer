#version 450

layout (location = 0) in vec2 vary_uv0;
layout (location = 0) out vec4 out_color;

layout (binding = 2) uniform sampler2D mask_tex;

void main() 
{
    float mask = texture(mask_tex,vary_uv0).r;
    if(mask < 0.1f)
    {
        discard;
    }
    out_color = vec4(1.0, 0.0, 0.0, 1.0);
}