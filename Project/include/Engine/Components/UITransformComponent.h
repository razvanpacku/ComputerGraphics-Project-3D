#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Engine/DataStructures/Transform.h"

struct UITransformComponent
{
	// relative values are applied first then absolute values as an 'offset'

	glm::vec2 position{ 0.0f, 0.0f }; // in pixel screen space
	float rotation = 0.0f; // radians
	glm::vec2 scale{ 0.0f, 0.0f }; // in pixel screen space ( {1.0f, 1.0f} means a single pixel)

	glm::vec2 relativePosition = { 0.5f, 0.5f }; // normalized (0.0 - 1.0) relative to parent size
	glm::vec2 relativeSize = { 1.f, 1.f }; // normalized (0.0 - 1.0) relative to parent size
	glm::vec2 anchorPoint = { 0.5f, 0.5f };

	int16_t zOrder = 0; // for layering GUI elements

	glm::mat4 localMatrix{ 1.f };
	glm::mat4 worldMatrix{ 1.f };
	glm::vec2 worldSize{ 1.f, 1.f }; // computed world size relative to screen

	bool localDirty = false;

	UITransformComponent() = default;
};

#include "Engine/DataStructures/TransformFunctions.h"
