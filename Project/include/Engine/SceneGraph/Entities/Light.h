#pragma once
#include "../Entity.h"
#include "Engine/Components/LightComponent.h"

// forward declarations
class RenderSystem;

// ======================================================
// Light
//
// Represents a light source in the scene.
// ======================================================
class Light : public Entity
{
public:
	Light(const std::string& name = "Light");

	void SetLightPosition(const glm::vec4& position);
	void SetLightColor(const glm::fixed_vec3& color);
	void SetAmbientStrength(float strength);
	void SetAttenuationFactor(const glm::fixed_vec3& factor);
	void SetLight(const LightingUBO& lightData);

private:
	LightComponent* lightComponent;
	friend class RenderSystem;
};

