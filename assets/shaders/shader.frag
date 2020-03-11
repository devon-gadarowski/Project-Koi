#version 450

#define MAX_LIGHT_COUNT 5

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(set = 1, binding = 0) uniform Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	int shininess;
	float opacity;
}inMaterial;

layout(set = 1, binding = 1) uniform sampler2D diffuseTexture;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in DirectionalLight inDirLight;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 lightDir = normalize(inDirLight.direction - inPos);
	vec4 diffuseColor = texture(diffuseTexture, inTexCoord);

	// AMBIENT
	vec3 ambient = inDirLight.ambient * vec3(diffuseColor);

	// DIFFUSE
	vec3 norm = normalize(inNormal);
	float diff = clamp(dot(lightDir, norm), 0.0, 1.0);
	vec3 diffuse = inDirLight.diffuse * (diff * vec3(diffuseColor));

	// SPECULAR
	float specularIntensity = 0.8;
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(normalize(-inPos), reflectDir), 0.0), inMaterial.shininess);
	vec3 specular = inDirLight.specular * (spec * inMaterial.specular);

	// OPACITY
	vec4 opacity = vec4(1.0, 1.0, 1.0, inMaterial.opacity);

	outColor = vec4((ambient + diffuse + specular), 1.0) * opacity;
}
