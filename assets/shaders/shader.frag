#version 450

layout(binding = 1) uniform sampler2D texSampler[1024];

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in float inOpacity;
layout(location = 4) flat in uint inTexID;
layout(location = 5) in vec3 inAmbient;
layout(location = 6) in vec3 inDiffuse;
layout(location = 7) in vec3 inSpecular;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 sunDirection = {1.0, -1.0, 0.0};

	vec4 opacity = vec4(1.0, 1.0, 1.0, inOpacity);
	vec4 ambient = vec4(0.2, 0.2, 0.2, 0.0) * texture(texSampler[inTexID], inTexCoord);
	vec4 diffuse = normalize(dot(-sunDirection, inNormal)) * texture(texSampler[inTexID], inTexCoord);
	vec4 specular = normalize(dot(-sunDirection, inNormal)) * vec4(inSpecular, 0.0);

	outColor = (diffuse + ambient + specular) * opacity;
}
