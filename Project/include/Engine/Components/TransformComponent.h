#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Engine/DataStructures/Transform.h"

struct TransformComponent
{
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::quat rotation = glm::quat({ 0.f, 0.f, 0.f });
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

	glm::mat4 localMatrix{ 1.f };
	glm::mat4 worldMatrix{ 1.f };

	bool localDirty = false;

	TransformComponent() = default;
	TransformComponent(const Transform& t) : position(t.position), rotation(t.rotation), scale(t.scale), localDirty(false) {}
};

#include "Engine/DataStructures/TransformFunctions.h"