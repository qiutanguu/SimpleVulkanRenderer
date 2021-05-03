glslc.exe ../source/tonemapper.frag -o ./spv/tonemapper_frag.spv
glslc.exe ../source/gbuffer.frag -o ./spv/gbuffer_frag.spv
glslc.exe ../source/lighting.frag -o ./spv/lighting_frag.spv
glslc.exe ../source/texture.frag -o ./spv/texture_frag.spv
glslc.exe ../source/gbuffer.vert -o ./spv/gbuffer_vert.spv
glslc.exe ../source/lighting.vert -o ./spv/lighting_vert.spv
glslc.exe ../source/shadowdepth.vert -o ./spv/shadowdepth_vert.spv
glslc.exe ../source/shadowdepth.frag -o ./spv/shadowdepth_frag.spv

cmd /k