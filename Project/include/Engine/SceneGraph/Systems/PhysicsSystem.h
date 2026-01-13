#pragma once
#include "../ISystem.h"

#include "Engine/Components/RigidBodyComponent.h"

class PhysicsSystem : public ISystem
{
public:
	PhysicsSystem(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	void OnUpdate(double deltaTime) override;

	virtual std::string GetName() const override { return "PhysicsSystem"; }

	void GiveRigidBody(Entity* entity, const RigidBodyInitData& rigidBodyData, float mass = 1.0f, bool anchored = false);
private:
	entt::registry* registry;
};

