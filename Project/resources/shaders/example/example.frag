#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

in vec2 ex_TexCoord;
in vec3 ex_Normal;
in vec3 fragPos;

out vec4 out_Color;

uniform sampler2D albedo;
uniform sampler2D metalness;

uniform sampler2DArrayShadow dirShadow;
uniform samplerCubeShadow pointShadow;

uniform bool receiveShadows;

layout (std140) uniform Camera {
	FIXED_VEC3 viewPos;
	mat4 view;
	mat4 projection;
};

layout(std140) uniform Lighting {
	vec4 lightPos;
	FIXED_VEC3 lightColor;
	float ambientStrength;
	FIXED_VEC3 attenuationFactor;
};

layout (std140) uniform Shadow {
	mat4 LightSpace[6];
    vec4 LightPos;
    float CascadedSplits[6];
};

layout(std140) uniform Material {
	float shininess;
	float specularStrength;
	float metalicity;
	bool overrideMetalness;
};

struct CascadeInfo {
    int index;
    int nextIndex;
    float blend;
};

const float CASCADE_BLEND_RANGE = 0.05;

CascadeInfo SelectCascadeBlended(vec3 fragPosWorld)
{
    float viewDepth = - (view * vec4(fragPosWorld, 1.0)).z;

    CascadeInfo info;
    info.index = 0;
    info.nextIndex = 0;
    info.blend = 0.0;

    for (int i = 0; i < 6; i++)
    {
        float splitStart = (i == 0) ? 0.1f : CascadedSplits[i - 1];
        float splitEnd   = CascadedSplits[i];
        float splitSize  = splitEnd - splitStart;

        if (viewDepth < splitEnd)
        {
            info.index = i;

            float blendStart = splitEnd - splitSize * CASCADE_BLEND_RANGE;

            if (viewDepth > blendStart && i < 3)
            {
                info.nextIndex = i + 1;
                info.blend = smoothstep(
                    blendStart,
                    splitEnd,
                    viewDepth
                );
            }
            else
            {
                info.nextIndex = i;
                info.blend = 0.0;
            }

            return info;
        }
    }

    info.index = 3;
    info.nextIndex = 3;
    info.blend = 0.0;
    return info; 
}

float SampleCascadeShadow(
    int cascade,
    vec3 fragPosWorld,
    vec3 normal
)
{
    vec4 fragPosLightSpace = LightSpace[cascade] * vec4(fragPosWorld, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float depth = projCoords.z;

    float dzdx = dFdx(depth);
    float dzdy = dFdy(depth);

    vec2 texelSize = 1.0 / vec2(textureSize(dirShadow, 0));

    float planarBias = abs(dzdx) * texelSize.x +
                       abs(dzdy) * texelSize.y;

    vec3 lightDir = normalize(lightPos.xyz);
    float slopeBias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0005);

    float cascadeScale = CascadedSplits[cascade] / CascadedSplits[5];
    float bias = (planarBias + slopeBias) * cascadeScale;

    return 1.0 - texture(
        dirShadow,
        vec4(projCoords.xy, cascade, depth - bias)
    );
}

float ShadowCalculationBlended(vec3 fragPosWorld, vec3 normal)
{
    CascadeInfo info = SelectCascadeBlended(fragPosWorld);

    float shadow0 = SampleCascadeShadow(info.index, fragPosWorld, normal);

    if (info.index == info.nextIndex)
        return shadow0;

    float shadow1 = SampleCascadeShadow(info.nextIndex, fragPosWorld, normal);

    return mix(shadow0, shadow1, info.blend);
}

float ShadowCalculationPointLight(vec3 fragPos)
{
    vec3 lightToFrag = fragPos - LightPos.xyz;
    float currentDepth = length(lightToFrag);

	vec3 norm = normalize(ex_Normal);
	vec3 lightDir = normalize(lightPos.xyz - fragPos);

	float minBias   = 0.01;   // constant bias
    float slopeBias = 0.005;   // slope-based bias

    float bias = max(
        slopeBias * (1.0 - dot(norm, lightDir)),
        minBias
    );

	float depth = currentDepth / CascadedSplits[0];

	bias *= depth;

    float shadow = 1.0 - texture(pointShadow,
                                 vec4(lightToFrag, depth - bias));
    return shadow;
}

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

	// shadow
	float shadow = 0.0;
	if(receiveShadows){
		if (lightPos.w == 0.0) {
			shadow = ShadowCalculationBlended(fragPos, norm);
		}
		else{
			shadow = ShadowCalculationPointLight(fragPos);
		}
	}

	// Combine all components

	vec3 diffuseComponent = diffuse * surfaceColor * (1.0 - realMetalicity);
	vec3 specularComponent = specular * mix(vec3(1.0f), surfaceColor, realMetalicity);
	vec4 result = vec4(ambient, 1.0f) * TexColor + vec4(attenuation * (diffuseComponent + specularComponent) * (1.0 - shadow), 1.0f);

	out_Color = result;
}