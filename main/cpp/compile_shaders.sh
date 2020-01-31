SHADER_LOCATION=../assets/shaders

rm -f $SHADER_LOCATION/frag.spv
rm -f $SHADER_LOCATION/vert.spv

$VULKAN_SDK/bin/glslc $SHADER_LOCATION/shader.vert -o $SHADER_LOCATION/vert.spv
$VULKAN_SDK/bin/glslc $SHADER_LOCATION/shader.frag -o $SHADER_LOCATION/frag.spv
