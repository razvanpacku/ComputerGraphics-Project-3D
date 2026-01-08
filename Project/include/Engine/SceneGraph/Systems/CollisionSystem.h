#pragma once
#include "../ISystem.h"

#include "Engine/Components/ColliderComponent.h"

//forward declaration
class TransformEntity;


struct Contact {
	TransformEntity* a;
	TransformEntity* b;
	glm::vec3 point;
	glm::vec3 normal;
	float penetration;
};

class CollisionSystem : public ISystem
{
public:
	CollisionSystem(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	void OnUpdate(double deltaTime) override;

	virtual std::string GetName() const override { return "CollisionSystem"; }

	void GiveCollisionShape(Entity* entity, const RigidBodyInitData& rigidBodyData, float mass = 1.0f, bool anchored = false);
private:
	entt::registry* registry;

	void BroadPhase();
	void NarrowPhase();
	void ResolveContacts(double deltaTime);

	std::vector<std::pair<entt::entity, entt::entity>> candidatePairs;
	std::vector<Contact> contacts;
};

