#pragma once
#include "../Entity.h"

#include "Engine/Components/TransformComponent.h"

// ================================================================
// TransformEntity
//
// An entity that has physically exists in the scene and can be transformed
// ================================================================
class TransformEntity : public virtual Entity
{
public:
	TransformEntity(const std::string& name = "");

	void SetLocalPosition(const glm::vec3& position);
	void SetLocalRotation(const glm::quat& rotation);
	void SetLocalScale(const glm::vec3& scale);

	glm::vec3 GetLocalPosition() const { return transformComponent->position; }
	glm::quat GetLocalRotation() const { return transformComponent->rotation; }
	glm::vec3 GetLocalScale() const { return transformComponent->scale; }

	void SetGlobalPosition(const glm::vec3& position);
	void SetGlobalRotation(const glm::quat& rotation);
	void SetGlobalScale(const glm::vec3& scale);

	glm::vec3 GetGlobalPosition() const;
	glm::quat GetGlobalRotation() const;
	glm::vec3 GetGlobalScale() const;
protected:
	TransformComponent* transformComponent = nullptr;
};

