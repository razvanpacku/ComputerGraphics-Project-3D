#include "Engine/SceneGraph//Entities/UITransformEntity.h"
#include "Engine/SceneGraph/Systems/UITransformSystem.h"

// ======================================================
// UITransformEntity
// ======================================================

UITransformEntity::UITransformEntity(const std::string& name)
	: Entity(name)
{
	uiTransformComponent = &AddComponent<UITransformComponent>();
}

void UITransformEntity::SetRelativePosition(const glm::vec2& position)
{
	uiTransformComponent->relativePosition = position;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}

void UITransformEntity::SetRotation(float rotation)
{
	uiTransformComponent->rotation = rotation;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}

void UITransformEntity::SetRelativeScale(const glm::vec2& scale)
{
	uiTransformComponent->relativeSize = scale;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}

void UITransformEntity::SetAbsolutePositionOffset(const glm::vec2& offset)
{
	uiTransformComponent->position = offset;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}

void UITransformEntity::SetAbsoluteScaleOffset(const glm::vec2& offset)
{
	uiTransformComponent->scale = offset;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}

void UITransformEntity::SetAnchorPoint(const glm::vec2& anchor)
{
	uiTransformComponent->anchorPoint = anchor;
	uiTransformComponent->localDirty = true;
	GetSystem<UITransformSystem>()->MarkDirty(GetHandle());
}


glm::vec2 UITransformEntity::GetPixelScale() const
{
	// make sure transform is up to date
	auto* uiTransformSystem = GetSystem<UITransformSystem>();
	uiTransformSystem->UpdateEntity(GetHandle());
	return uiTransformComponent->worldSize * glm::vec2((float)uiTransformSystem->ScreenWidth(), (float)uiTransformSystem->ScreenHeight());
}