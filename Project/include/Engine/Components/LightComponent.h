#pragma once
#include "Engine/Resources/UboDefs.h"

struct LightComponent : public LightingUBO {
	LightComponent() {
		lightPos = glm::aligned_vec4(0.0f, 1.0f, 0.0f, 0.0f);
		lightColor = glm::fixed_vec3(1.0f, 1.0f, 1.0f);
		ambientStrength = 0.1f;
		attenuationFactor = glm::fixed_vec3(1.0f, 0.09f, 0.032f);
	}
	bool dirty = false;
};
