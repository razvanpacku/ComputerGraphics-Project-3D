#include "Engine/SceneGraph/Systems/UITransformSystem.h"

#include "Engine/SceneGraph/Systems/RenderSystem.h"

#include "Engine/App.h"

// ======================================================
// UITransformSystem
// ======================================================

UITransformSystem::UITransformSystem(Scene* scene, int16_t order, entt::registry* registry)
	: TransformSystemBase(scene, order, registry)
{
}

void UITransformSystem::OnUpdate(double deltaTime)
{
	// Check for screen size change
	int currentScreenWidth = ScreenWidth();
	int currentScreenHeight = ScreenHeight();
	if (currentScreenWidth != oldScreenWidth || currentScreenHeight != oldScreenHeight) {
		// Screen size changed, mark all UITransformComponents as dirty
		auto view = registry->view<UITransformComponent>();
		for (auto entity : view) {
			MarkDirty(entity);
		}
		oldScreenWidth = currentScreenWidth;
		oldScreenHeight = currentScreenHeight;
	}
	// Call base class OnUpdate to handle dirty transforms
	TransformSystemBase::OnUpdate(deltaTime);
}

void UITransformSystem::UpdateTransform(entt::entity entity)
{
	auto* uiTransformC = registry->try_get<UITransformComponent>(entity);
	if (uiTransformC) {
		//get parent world size
		glm::vec2 parentWorldSize = glm::vec2(1.0f);
		auto* hierC = registry->try_get<HierarchyComponent>(entity);
		if (hierC && hierC->parent != entt::null) {
			auto* parentTransformC = registry->try_get<UITransformComponent>(hierC->parent);
			if (parentTransformC) {
				parentWorldSize = parentTransformC->worldSize;
			}
		}
		


		// Update local matrix
		uiTransformC->localMatrix = TransformFunctions::UIComputeLocal(
			uiTransformC->position,
			uiTransformC->relativePosition,
			uiTransformC->scale,
			uiTransformC->relativeSize,
			uiTransformC->rotation,
			uiTransformC->anchorPoint,
			parentWorldSize,
			{ (float)ScreenWidth(), (float)ScreenHeight() }
		);
		glm::vec2 worldRelSize =
			uiTransformC->relativeSize * parentWorldSize +
			uiTransformC->scale / glm::vec2(ScreenWidth(), ScreenHeight());

		uiTransformC->worldSize = worldRelSize;

		uiTransformC->localDirty = false;

		// Update world matrix
		if (hierC && hierC->parent != entt::null) {
			auto* parentTransformC = registry->try_get<UITransformComponent>(hierC->parent);
			if (parentTransformC) {
				uiTransformC->worldMatrix = TransformFunctions::ComputeGlobal(parentTransformC->worldMatrix, uiTransformC->localMatrix);
				uiTransformC->zOrder = parentTransformC->zOrder + 1;
			}
			else {
				uiTransformC->worldMatrix = uiTransformC->localMatrix;
				uiTransformC->zOrder = 0;
			}
		}
		else {
			uiTransformC->worldMatrix = uiTransformC->localMatrix;
			uiTransformC->zOrder = 0;
		}

		// check if entity is renderable and update its transforms
		auto* renderableC = registry->try_get<RenderableComponent>(entity);
		if (renderableC) {
			renderableC->UpdateTransform(uiTransformC->worldMatrix);
			renderableC->UpdateZOrder(uiTransformC->zOrder);
		}

		// Remove dirty tag
		registry->remove<UITransformDirtyTag>(entity);
	}
}

int UITransformSystem::ScreenWidth() const
{
	return App::Get().GetWindowWidth();
}

int UITransformSystem::ScreenHeight() const
{
	return App::Get().GetWindowHeight();
}