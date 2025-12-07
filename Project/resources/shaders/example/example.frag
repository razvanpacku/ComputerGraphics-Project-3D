#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

in vec2 ex_TexCoord;
in vec3 ex_Normal;
in vec3 fragPos;

out vec4 out_Color;

uniform sampler2D albedo;
uniform sampler2D metalness;

layout(std140) uniform Camera {
	FIXED_VEC3 viewPos;
};

layout(std140) uniform Lighting {
	vec4 lightPos;
	FIXED_VEC3 lightColor;
	float ambientStrength;
	FIXED_VEC3 attenuationFactor;
};

layout(std140) uniform Material {
	float shininess;
	float specularStrength;
	float metalicity;
	bool overrideMetalness;
};

void main(void){
	FIXED_VEC3_INIT(viewPos);
	FIXED_VEC3_INIT(lightColor);
	FIXED_VEC3_INIT(attenuationFactor);

	float tile = 1.0;
	vec4 TexColor = texture(albedo, ex_TexCoord * tile);
	vec3 surfaceColor = vec3(TexColor);

	vec3 norm = normalize(ex_Normal);

	// Determine light direction
	vec3 lightDir;
	if (lightPos.w == 0.0) {
		// Directional light
		lightDir = normalize(lightPos.xyz);
	}
	else {
		// Point light
		lightDir = normalize(lightPos.xyz - fragPos);
	}
	// Ambient component
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse component
	float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	// Specular component
	vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	if(diff <= 0.0) {
		spec = 0;
	}
    vec3 specular = specularStrength * spec * lightColor;

	// Attenuation component (for point lights)
	float attenuation = 1.0;
	if(lightPos.w != 0.0) {
		float distance = length(lightPos.xyz - fragPos);
		attenuation = 1.0 / (attenuationFactor.x + attenuationFactor.y * distance + attenuationFactor.z * (distance * distance));
	}

	//metallness
	float realMetalicity;

	if(overrideMetalness) {
		realMetalicity = metalicity;
	}
	else{
		vec4 MetColor = texture(metalness, ex_TexCoord);
		realMetalicity = (MetColor.r + MetColor.g + MetColor.b) / 3;
	}

	// Combine all components

	vec3 diffuseComponent = diffuse * surfaceColor * (1.0 - realMetalicity);
	vec3 specularComponent = specular * mix(vec3(1.0f), surfaceColor, realMetalicity);
	vec4 result = vec4(ambient, 1.0f) * TexColor + vec4(attenuation * (diffuseComponent + specularComponent), 1.0f);

	out_Color = result;
}