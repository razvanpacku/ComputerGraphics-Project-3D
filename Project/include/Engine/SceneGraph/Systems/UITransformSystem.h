#pragma once
#include <entt/entt.hpp>
#include "TransformSystemBase.h"

#include "Engine/Components/UITransformComponent.h"

class UITransformSystem : public TransformSystemBase<UITransformComponent, UITransformDirtyTag>
{
public:
	UITransformSystem(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	~UITransformSystem() override = default;
	void OnUpdate(double deltaTime) override;

	void UpdateTransform(entt::entity entity) override;
	std::string GetName() const override { return "UITransformSystem"; }

	int ScreenWidth() const;
	int ScreenHeight() const;

private:

	int oldScreenWidth = 0;
	int oldScreenHeight = 0;
};

