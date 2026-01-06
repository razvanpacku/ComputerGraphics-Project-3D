#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Forward declarations
struct Transform;
struct TransformComponent;

namespace TransformFunctions {
	glm::mat4 ComputeLocal(
		const glm::vec3& pos,
		const glm::quat& rot,
		const glm::vec3& sca
	);

	glm::mat4 ComputeGlobal(
		const glm::mat4& local,
		const glm::mat4& parentGlobal
	);

	glm::vec3 DecomposePosition(const glm::mat4& matrix);
	glm::quat DecomposeRotation(const glm::mat4& matrix);
	glm::vec3 DecomposeScale(const glm::mat4& matrix);

	void Decompose(TransformComponent& transformComponent, const glm::mat4& localMatrix);
}