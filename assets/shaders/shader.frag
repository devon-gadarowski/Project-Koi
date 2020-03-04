#version 450

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

layout(location = 0) in vec3 inLightPos;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = texture(diffuseTexture, inTexCoord);

	// AMBIENT
	float ambient = 0.1;

	// DIFFUSE
	vec3 lightDir = normalize(inLightPos - inPos);
	vec3 norm = normalize(inNormal);
	float diffuse = clamp(dot(lightDir, norm), 0.0, 1.0);

	// SPECULAR
	float specularIntensity = 0.8;
	vec3 reflectDir = reflect(-lightDir, norm);
	float specular = pow(max(dot(normalize(-inPos), reflectDir), 0.0), inMaterial.shininess);

	// OPACITY
	vec4 opacity = vec4(1.0, 1.0, 1.0, inMaterial.opacity);

	outColor = (ambient + diffuse + specular) * color * opacity;
}
