#version 450

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 4) in float inOpacity;
layout(location = 5) in uint inTexID;
layout(location = 6) in vec3 inAmbient;
layout(location = 7) in vec3 inDiffuse;
layout(location = 8) in vec3 inSpecular;
layout(location = 9) in mat4 inModelMat;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out float outOpacity;
layout(location = 5) out uint outTexID;
layout(location = 6) out vec3 outAmbient;
layout(location = 7) out vec3 outDiffuse;
layout(location = 8) out vec3 outSpecular;
layout(location = 9) out vec3 outLightPos;

void main()
{
	gl_Position = ubo.proj * (ubo.view * (inModelMat * vec4(inPosition, 1.0)));

	outPos = vec3(ubo.view * (inModelMat * vec4(inPosition, 1.0)));
	outColor = inColor;
	outTexCoord = inTexCoord;
	outNormal = vec3((mat4(transpose(inverse(ubo.view * inModelMat))) * vec4(inNormal, 1.0)));
	outOpacity = inOpacity;
	outTexID = inTexID;
	outAmbient = inAmbient;
	outDiffuse = inDiffuse;
	outSpecular = inSpecular;

	outLightPos = vec3(ubo.view * (vec4(2.0, 2.0, -2.0, 1.0)));
}
