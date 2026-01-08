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
		// Store old scale
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