#version 450

#define MAX_LIGHT_COUNT 5

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(set = 0, binding = 0) uniform Camera
{
	mat4 view;
	mat4 proj;
} camera;

layout(set = 0, binding = 1) uniform inDirectionalLight
{
	DirectionalLight inDirLight;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in mat4 inModelMat;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out DirectionalLight outDirLight;

void main()
{
	gl_Position = camera.proj * (camera.view * (inModelMat * vec4(inPosition, 1.0)));

	outPos = vec3(camera.view * (inModelMat * vec4(inPosition, 1.0)));
	outTexCoord = inTexCoord;
	outNormal = vec3((mat4(transpose(inverse(camera.view * inModelMat))) * vec4(inNormal, 1.0)));

	outDirLight = inDirLight;
	outDirLight.direction = vec3(camera.view * vec4(inDirLight.direction, 1.0));
}
