#include "Engine/DataStructures/TransformFunctions.h"

#include "Engine/Components/TransformComponent.h"

namespace TransformFunctions {
	glm::mat4 ComputeLocal(
		const glm::vec3& pos,
		const glm::quat& rot,
		const glm::vec3& sca
	) {
		glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 R = glm::mat4_cast(rot);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), sca);
		return T * R * S;
	}

	glm::mat4 ComputeGlobal(
		const glm::mat4& parentGlobal,
		const glm::mat4& local
	) {
		return parentGlobal * local;
	}

	glm::vec3 DecomposePosition(const glm::mat4& matrix) {
		return glm::vec3(matrix[3]);
	}

	glm::quat DecomposeRotation(const glm::mat4& matrix) {
		glm::vec3 scale = DecomposeScale(matrix);
		glm::mat4 rotationMatrix = matrix;
		// Remove scaling from the rotation matrix
		rotationMatrix[0] /= scale.x;
		rotationMatrix[1] /= scale.y;
		rotationMatrix[2] /= scale.z;
		return glm::quat_cast(rotationMatrix);
	}

	glm::vec3 DecomposeScale(const glm::mat4& matrix) {
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(matrix[0]));
		scale.y = glm::length(glm::vec3(matrix[1]));
		scale.z = glm::length(glm::vec3(matrix[2]));
		return scale;
	}

	void Decompose(TransformComponent& transformComponent, const glm::mat4& localMatrix) {
		transformComponent.position = DecomposePosition(localMatrix);
		transformComponent.rotation = DecomposeRotation(localMatrix);
		transformComponent.scale = DecomposeScale(localMatrix);
	}
}