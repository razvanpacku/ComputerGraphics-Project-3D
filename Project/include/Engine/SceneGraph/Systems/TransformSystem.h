#pragma once
#include <entt/entt.hpp>
#include "TransformSystemBase.h"

#include "Engine/Components/TransformComponent.h"

class TransformSystem : public TransformSystemBase<TransformComponent, TransformDirtyTag>
{
public:
	TransformSystem(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	~TransformSystem() override = default;
	void UpdateTransform(entt::entity entity) override;
	virtual std::string GetName() const override { return "TransformSystem"; }

	void SetTarget(entt::entity entity); // sets the target entity for the camera to follow
};