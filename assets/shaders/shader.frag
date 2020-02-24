#version 450

layout(binding = 1) uniform sampler2D texSampler[1024];

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in float inOpacity;
layout(location = 5) flat in uint inTexID;
layout(location = 6) in vec3 inAmbient;
layout(location = 7) in vec3 inDiffuse;
layout(location = 8) in vec3 inSpecular;
layout(location = 9) in vec3 inLightPos;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	vec4 objectColor = texture(texSampler[inTexID], inTexCoord);

	// AMBIENT
	float ambientIntensity = 0.1;
	vec3 ambient = ambientIntensity * lightColor;

	// DIFFUSE
	vec3 lightDir = normalize(inLightPos - inPos);
	vec3 norm = normalize(inNormal);
	vec3 diffuse = clamp(dot(lightDir, norm), 0.0, 1.0) * lightColor;

	// SPECULAR
	float specularIntensity = 0.8;
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 specular = pow(max(dot(normalize(-inPos), reflectDir), 0.0), 32) * specularIntensity * lightColor;

	// OPACITY
	vec4 opacity = vec4(1.0, 1.0, 1.0, inOpacity);

	outColor = vec4(ambient + diffuse + specular, opacity) * objectColor;
}
