#version 450

layout(set = 1, binding = 0) uniform Material
{
	bool ka;
	bool kd;
	bool ks;
	bool norm;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	int shininess;
	float opacity;
}inMaterial;

layout(set = 1, binding = 1) uniform sampler2D texSampler[4];

layout(location = 0) in vec3 inLightPos;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	int texID = 0;

	// AMBIENT
	if (inMaterial.ka == true)
		ambient = vec3(texture(texSampler[0], inTexCoord));
	else if (inMaterial.kd == true)
		ambient = vec3(texture(texSampler[0], inTexCoord));
	else
		ambient = inMaterial.ambient;

	// DIFFUSE
	vec3 lightDir = normalize(inLightPos - inPos);
	vec3 norm = normalize(inNormal);
	float diffuseStr = clamp(dot(lightDir, norm), 0.0, 1.0);
	if (inMaterial.kd == true)
		diffuse = diffuseStr * vec3(texture(texSampler[0], inTexCoord));
	else
		diffuse = diffuseStr * inMaterial.diffuse;

	// SPECULAR
	float specularIntensity = 0.8;
	vec3 reflectDir = reflect(-lightDir, norm);
	float specularStr = pow(max(dot(normalize(-inPos), reflectDir), 0.0), inMaterial.shininess);
	if (inMaterial.ks == true)
		specular = specularStr * vec3(texture(texSampler[0], inTexCoord));
	else
		specular = specularStr * inMaterial.specular;

	// OPACITY
	float opacity = inMaterial.opacity;

	// FOG
	//vec3 fogColor = vec3(1.0, 1.0, 1.0);
	//float fogFactor = clamp((sqrt(dot(inPos, inPos)) - 40.0) / 10.0, 0.0, 1.0);
	//vec4 fog = vec4(fogFactor * fogColor, 1.0);

	outColor = vec4(ambient + diffuse + specular, opacity);// + fog;
}
