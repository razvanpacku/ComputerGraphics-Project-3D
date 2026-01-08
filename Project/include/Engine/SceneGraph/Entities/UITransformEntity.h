#pragma once
#include "../Entity.h"

#include "Engine/Components/UITransformComponent.h"

// ======================================================
// UITransformEntity
//
// Represents a UI element with transform properties.
// ======================================================
class UITransformEntity : public virtual Entity
{
public:
	UITransformEntity(const std::string& name = "");

	void SetRelativePosition(const glm::vec2& position);
	void SetRotation(float rotation);
	void SetRelativeScale(const glm::vec2& scale);
	void SetAbsolutePositionOffset(const glm::vec2& offset);
	void SetAbsoluteScaleOffset(const glm::vec2& offset);
	void SetAnchorPoint(const glm::vec2& anchor);

	glm::vec2 GetRelativePosition() const { return uiTransformComponent->relativePosition; }
	float GetRotation() const { return uiTransformComponent->rotation; }
	glm::vec2 GetRelativeScale() const { return uiTransformComponent->relativeSize; }
	glm::vec2 GetAbsolutePositionOffset() const { return uiTransformComponent->position; }
	glm::vec2 GetAbsoluteScaleOffset() const { return uiTransformComponent->scale; }
	glm::vec2 GetAnchorPoint() const { return uiTransformComponent->anchorPoint; }

	glm::vec2 GetPixelScale() const;
protected:
	UITransformComponent* uiTransformComponent = nullptr;
};

