#pragma once
#include <entt/entt.hpp>
#include "../ISystem.h"

#include "Engine/Components/TransformComponent.h"

class TransformSystem : public ISystem
{
public:
	TransformSystem(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	~TransformSystem() override = default;
	void OnUpdate(double deltaTime) override;

	bool MarkDirty(entt::entity entity);

	void UpdateTransform(entt::entity entity);
	void UpdateEntity(entt::entity entity);

	virtual std::string GetName() const override { return "TransformSystem"; }
private:
	entt::registry* registry = nullptr;

	void UpdateSubtree(entt::entity entity);
};

