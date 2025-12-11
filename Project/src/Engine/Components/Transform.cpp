#include "Engine/Components/Transform.h"

// =================================================
// Transform
// =================================================
glm::mat4 Transform::GetModelMatrix() const {
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::toMat4(rotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
	return translationMatrix * rotationMatrix * scaleMatrix;
}