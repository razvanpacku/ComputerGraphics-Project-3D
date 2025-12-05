#version 460

in vec2 ex_TexCoord;
in vec3 ex_Normal;
in vec3 fragPos;

out vec4 out_Color;

uniform sampler2D Texture;

layout (std140) uniform Test {
	mat4 first;
	vec3 testColor[2];
};

uniform vec3 viewPos;

vec4 lightPos = vec4(1.0f, 0.0f, 0.0f, 1.0f);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float ambientStrength = 0.25;
float shininess = 4.0;
float specularStrength = 1.0f;
vec3 attenuationFactor = vec3(1.0, 0.09, 0.32);

float metalicity = 0.0f;

void main(void){
	float tile = 4.0;
	vec4 TexColor = texture(Texture, ex_TexCoord * tile);
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
    vec3 specular = specularStrength * spec * lightColor;

	// Attenuation component (for point lights)
	float attenuation = 1.0;
	if(lightPos.w != 0.0) {
		float distance = length(lightPos.xyz - fragPos);
		attenuation = 1.0 / (attenuationFactor.x + attenuationFactor.y * distance + attenuationFactor.z * (distance * distance));
	}

	// Combine all components

	vec3 diffuseComponent = diffuse * surfaceColor * (1.0 - metalicity);
	vec3 specularComponent = specular * mix(vec3(1.0f), surfaceColor, metalicity);
	vec4 result = vec4(ambient, 1.0f) * TexColor + vec4(attenuation * (diffuseComponent + specularComponent), 1.0f);

	out_Color = result;
}