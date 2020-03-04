#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in mat4 inModelMat;

layout(location = 0) out vec3 outLightPos;
layout(location = 1) out vec3 outPos;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;

void main()
{
	gl_Position = ubo.proj * (ubo.view * (inModelMat * vec4(inPosition, 1.0)));

	outPos = vec3(ubo.view * (inModelMat * vec4(inPosition, 1.0)));
	outTexCoord = inTexCoord;
	outNormal = vec3((mat4(transpose(inverse(ubo.view * inModelMat))) * vec4(inNormal, 1.0)));

	outLightPos = vec3(ubo.view * (vec4(2.0, 2.0, -2.0, 1.0)));
}
