#include "Engine/SceneGraph/Systems/TransformSystem.h"

#include "Engine/Components/Components.h"
#include "Engine/DataStructures/TransformFunctions.h"

#include "Engine/SceneGraph/Systems/RenderSystem.h"

// ======================================================
// TransformSystem
// ======================================================

TransformSystem::TransformSystem(Scene* scene, int16_t order, entt::registry* registry)
	: TransformSystemBase(scene, order, registry)
{
}

void TransformSystem::UpdateTransform(entt::entity entity)
{
	auto* transformC = registry->try_get<TransformComponent>(entity);
	if (transformC) {
		// Store old position, rotation, scale
		glm::vec3 oldPosition = TransformFunctions::DecomposePosition(transformC->worldMatrix);
		glm::quat oldRotation = TransformFunctions::DecomposeRotation(transformC->worldMatrix);
		glm::vec3 oldScale = TransformFunctions::DecomposeScale(transformC->worldMatrix);

		// Update local matrix
		if(transformC->localDirty) {
			// If localDirty is true, we need to recompute local matrix
			// It can be false if only the ancestor's local/global matrix changed
			transformC->localMatrix = TransformFunctions::ComputeLocal(transformC->position, transformC->rotation, transformC->scale);
			transformC->localDirty = false;
		}

		// Update world matrix
		auto* hierC = registry->try_get<HierarchyComponent>(entity);
		if (hierC && hierC->parent != entt::null) {
			auto* parentTransformC = registry->try_get<TransformComponent>(hierC->parent);
			if (parentTransformC) {
				transformC->worldMatrix = TransformFunctions::ComputeGlobal(parentTransformC->worldMatrix, transformC->localMatrix);
			}
			else {
				transformC->worldMatrix = transformC->localMatrix;
			}
		}
		else {
			transformC->worldMatrix = transformC->localMatrix;
		}

		// check if entity has a rigidbody component and update its intertia tensor with new scale
		auto* rigidbodyC = registry->try_get<RigidBodyComponent>(entity);
		if (rigidbodyC) {
			glm::vec3 nonUniformScale = TransformFunctions::DecomposeScale(transformC->worldMatrix);
			float s = glm::length(nonUniformScale / oldScale) / sqrt(3.0f); // average scale factor
			rigidbodyC->inertiaTensor *= s * s;
			rigidbodyC->inverseInertiaTensor /= s * s;

			// skip lag spikes
			if (rigidbodyC->anchored && deltaTime > 0) {
				// compute velocity and angular velocity based on change in position and rotation
				glm::vec3 newPosition = TransformFunctions::DecomposePosition(transformC->worldMatrix);
				glm::quat newRotation = TransformFunctions::DecomposeRotation(transformC->worldMatrix);

				if (oldPosition == newPosition) {
					rigidbodyC->velocity = glm::vec3(0.0f);
				}
				else {
					glm::vec3 linearVelocity = (newPosition - oldPosition) / static_cast<float>(deltaTime);
					rigidbodyC->velocity = linearVelocity;
				}
				// --- Angular velocity ---
				if (oldRotation == newRotation) {
					rigidbodyC->angularVelocity = glm::vec3(0.0f);
				}
				else {
					glm::quat dq = newRotation * glm::inverse(oldRotation);
					dq = glm::normalize(dq);

					float angle = 2.0f * acos(glm::clamp(dq.w, -1.0f, 1.0f));
					float sinHalf = sqrtf(1.0f - dq.w * dq.w);

					glm::vec3 axis;
					if (sinHalf < 1e-6f) {
						// rotation too small, approximate axis from quaternion xyz
						axis = glm::normalize(glm::vec3(dq.x, dq.y, dq.z));
					}
					else {
						axis = glm::vec3(dq.x, dq.y, dq.z) / sinHalf;
					}

					glm::vec3 angularVelocity = axis * (angle / static_cast<float>(deltaTime));
					rigidbodyC->angularVelocity = angularVelocity;
				}
			}
		}

		// check if entity is renderable and update its transforms
		auto* renderableC = registry->try_get<RenderableComponent>(entity);
		if (renderableC) {
			renderableC->UpdateTransform(transformC->worldMatrix);
		}

		// If this entity is the target entity, notify the RenderSystem to update the camera position
		if (registry->all_of<TargetEntityTag>(entity)) {
			auto* renderSystem = GetSystem<RenderSystem>();
			if (renderSystem) {
				// extract position from worldMatrix
				glm::vec3 worldPosition = glm::vec3(transformC->worldMatrix[3]);
				renderSystem->UpdateTargetCamera(worldPosition);
			}
		}


		// Remove dirty tag
		registry->remove<TransformDirtyTag>(entity);
	}
}

void TransformSystem::SetTarget(entt::entity entity)
{
	// First, remove TargetEntityTag from any existing target entity
	auto view = registry->view<TargetEntityTag>();
	for (auto entt : view) {
		registry->remove<TargetEntityTag>(entt);
	}
	// Now, set the new target entity (if it has a TransformComponent)
	if (registry->all_of<TransformComponent>(entity)) {
		registry->emplace<TargetEntityTag>(entity);
	}
}