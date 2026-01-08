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

	// Awful, *awful*, very bad, very vibecoded; Awful.
	glm::mat4 UIComputeLocal(
		const glm::vec2& pixelPos,
		const glm::vec2& relativePos,
		const glm::vec2& pixelSize,
		const glm::vec2& relativeSize,
		float rotation,
		const glm::vec2& anchorPoint,
		const glm::vec2& parentWorldRelSize,
		const glm::vec2& screenSize
	) {
		// Compute final size in normalized space
		glm::vec2 size = relativeSize + pixelSize / (screenSize * parentWorldRelSize);

		// Compute the position in normalized space (anchor point will be placed here)
		glm::vec2 pos = relativePos + pixelPos / (screenSize * parentWorldRelSize) - glm::vec2(0.5f);

		//  Compute pivot offset in local quad space [-0.5,0.5]
		glm::vec2 pivotOffset = (anchorPoint - glm::vec2(0.5f));

		// Translate so the anchor sits at 'pos'
		glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(pos - pivotOffset, 0.0f));

		float aspect = (screenSize.x / screenSize.y) * (parentWorldRelSize.x / parentWorldRelSize.y);

		glm::mat4 aspectFix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, aspect, 1.0f));
		glm::mat4 invAspectFix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f / aspect, 1.0f));

		// Rotation around anchor (pivotOffset already applied in T)
		glm::mat4 R =
			glm::translate(glm::mat4(1.0f), glm::vec3(pivotOffset, 0.f)) *
			aspectFix *
			glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1)) *
			invAspectFix *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f)) *
			glm::translate(glm::mat4(1.0f), glm::vec3(-pivotOffset, 0.f));

		return T * R;
	}
}