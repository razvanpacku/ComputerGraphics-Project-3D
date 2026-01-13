#include "Engine/SceneGraph/Entities/Light.h"

// ======================================================
// Light
// =====================================================

Light::Light(const std::string& name)
	: Entity(name)
{
	lightComponent = &AddComponent<LightComponent>();
}

void Light::SetLightPosition(const glm::vec4& position)
{
	lightComponent->lightPos = glm::aligned_vec4(position);
	lightComponent->dirty = true;
}

void Light::SetLightColor(const glm::fixed_vec3& color)
{
	lightComponent->lightColor = color;
	lightComponent->dirty = true;
}

void Light::SetAmbientStrength(float strength)
{
	lightComponent->ambientStrength = strength;
	lightComponent->dirty = true;
}

void Light::SetAttenuationFactor(const glm::fixed_vec3& factor)
{
	lightComponent->attenuationFactor = factor;
	lightComponent->dirty = true;
}

void Light::SetLight(const LightingUBO& lightData)
{
	lightComponent->lightPos = lightData.lightPos;
	lightComponent->lightColor = lightData.lightColor;
	lightComponent->ambientStrength = lightData.ambientStrength;
	lightComponent->attenuationFactor = lightData.attenuationFactor;
	lightComponent->dirty = true;
}
