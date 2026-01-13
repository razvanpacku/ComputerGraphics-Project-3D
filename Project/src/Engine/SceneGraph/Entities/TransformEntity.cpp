#include "Engine/SceneGraph/Entities/TransformEntity.h"
#include "Engine/SceneGraph/Systems/TransformSystem.h"

// ================================================================
// TransformEntity
// ================================================================

TransformEntity::TransformEntity(const std::string& name)
	: Entity(name)
{
	transformComponent = &AddComponent<TransformComponent>();
}

void TransformEntity::SetLocalPosition(const glm::vec3& position)
{
	transformComponent->position = position;
	transformComponent->localDirty = true;
	GetSystem<TransformSystem>()->MarkDirty(GetHandle());
}

void TransformEntity::SetLocalRotation(const glm::quat& rotation)
{
	transformComponent->rotation = rotation;
	transformComponent->localDirty = true;
	GetSystem<TransformSystem>()->MarkDirty(GetHandle());
}

void TransformEntity::SetLocalScale(const glm::vec3& scale)
{
	transformComponent->scale = scale;
	transformComponent->localDirty = true;
	GetSystem<TransformSystem>()->MarkDirty(GetHandle());
}

void TransformEntity::SetGlobalPosition(const glm::vec3& position)
{
	// To set the global position, we need to consider the parent's world matrix.
	Entity* parent = GetParent();
	if (parent) {
		if (!parent->HasComponent<TransformComponent>()) {
			// No parent transform, treat as local.
			SetLocalPosition(position);
			return;
		}
		TransformComponent* parentTransform = &parent->GetComponent<TransformComponent>();

		GetSystem<TransformSystem>()->UpdateEntity(parent->GetHandle());

		glm::vec4 local = glm::inverse(parentTransform->worldMatrix) * glm::vec4(position, 1.0f);

		SetLocalPosition(glm::vec3(local));
	}
}

void TransformEntity::SetGlobalRotation(const glm::quat& rotation)
{
	// To set the global rotation, we need to consider the parent's world matrix.
	Entity* parent = GetParent();
	if (parent) {
		if (!parent->HasComponent<TransformComponent>()) {
			// No parent transform, treat as local.
			SetLocalRotation(rotation);
			return;
		}
		TransformComponent* parentTransform = &parent->GetComponent<TransformComponent>();

		GetSystem<TransformSystem>()->UpdateEntity(parent->GetHandle());

		glm::quat localRotation = glm::inverse(parentTransform->rotation) * rotation;

		SetLocalRotation(localRotation);

	}
}

void TransformEntity::SetGlobalScale(const glm::vec3& scale)
{
	// To set the global scale, we need to consider the parent's world matrix.
	Entity* parent = GetParent();
	if (parent) {
		if (!parent->HasComponent<TransformComponent>()) {
			// No parent transform, treat as local.
			SetLocalScale(scale);
			return;
		}
		TransformComponent* parentTransform = &parent->GetComponent<TransformComponent>();

		GetSystem<TransformSystem>()->UpdateEntity(parent->GetHandle());

		glm::vec3 parentScale = TransformFunctions::DecomposeScale(parentTransform->worldMatrix);
		glm::vec3 localScale = scale / parentScale;
		SetLocalScale(localScale);
	}
}

glm::vec3 TransformEntity::GetGlobalPosition() const
{
	// make sure the world matrix is up to date
	GetSystem<TransformSystem>()->UpdateEntity(GetHandle());
	return TransformFunctions::DecomposePosition(transformComponent->worldMatrix);
}

glm::quat TransformEntity::GetGlobalRotation() const
{
	// make sure the world matrix is up to date
	GetSystem<TransformSystem>()->UpdateEntity(GetHandle());
	return TransformFunctions::DecomposeRotation(transformComponent->worldMatrix);
}

glm::vec3 TransformEntity::GetGlobalScale() const
{
	// make sure the world matrix is up to date
	GetSystem<TransformSystem>()->UpdateEntity(GetHandle());
	return TransformFunctions::DecomposeScale(transformComponent->worldMatrix);
}