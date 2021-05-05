glslc.exe ./source/tonemapper.frag -o ./spir-v/tonemapper_frag.spv
glslc.exe ./source/gbuffer.frag -o ./spir-v/gbuffer_frag.spv
glslc.exe ./source/lighting.frag -o ./spir-v/lighting_frag.spv
glslc.exe ./source/texture.frag -o ./spir-v/texture_frag.spv
glslc.exe ./source/texture.vert -o ./spir-v/texture_vert.spv
glslc.exe ./source/gbuffer.vert -o ./spir-v/gbuffer_vert.spv
glslc.exe ./source/lighting.vert -o ./spir-v/lighting_vert.spv
glslc.exe ./source/shadowdepth.vert -o ./spir-v/shadowdepth_vert.spv
glslc.exe ./source/shadowdepth.frag -o ./spir-v/shadowdepth_frag.spv
glslc.exe ./source/uioverlay.vert -o ./spir-v/uioverlay_vert.spv
glslc.exe ./source/uioverlay.frag -o ./spir-v/uioverlay_frag.spv

cmd /k