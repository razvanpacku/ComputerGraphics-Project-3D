#include "Engine/SceneGraph/Entity.h"

#include "Engine/Components/NameComponent.h"

// ======================================================
// Entity
// ======================================================

uint64_t Entity::nextID = 0;

Entity::Entity(const std::string& name)
{
	scene = Scene::GetActiveScene();
	if (scene) {
		handle = scene->registry.create();

		// all entities get a NameComponent by default
		if(name == "") {
			//scene->registry.emplace<NameComponent>(handle, "Entity_" + std::to_string(nextID++));
			AddComponent<NameComponent>("Entity_" + std::to_string(nextID++));
		}
		else {
			AddComponent<NameComponent>(name);
		}
		// add hierarchy component
		scene->registry.emplace<HierarchyComponent>(handle);
	}
	else {
		handle = entt::null;
	}
}

Entity::~Entity()
{
	if (scene && handle != entt::null) {
		scene->registry.destroy(handle);
	}
}

Entity* const Entity::GetParent() const
{
	auto* hier = scene->registry.try_get<HierarchyComponent>(handle);
	if (hier && hier->parent != entt::null) {
		auto it = scene->entityMap.find(hier->parent);
		if (it != scene->entityMap.end()) {
			return it->second;
		}
	}
	return nullptr;
}

void Entity::SetParent(Entity* const parent)
{
	scene->AddOrMoveEntity(*this, parent);
}

std::vector<Entity*> Entity::GetChildren() const
{
	std::vector<Entity*> children;
	scene->GetChildren(*this, children);
	return children;
}

std::vector<Entity*> Entity::GetDescendants() const
{
	std::vector<Entity*> descendants;
	scene->GetDescendants(*this, descendants);
	return descendants;
}

Entity* const Entity::FindFirstChild(const std::string& name) const
{
	return scene->FindFirstChild(name, this);
}

Entity* const Entity::FindFirstDescendant(const std::string& name) const
{
	return scene->FindFirstDescendant(name, this);
}

void Entity::Destroy()
{
	scene->RemoveEntity(this);
}

std::ostream& operator<<(std::ostream& os, const Entity& entity)
{
	if (entity.scene && entity.handle != entt::null) {
		const auto& nameComp = entity.GetComponent<NameComponent>();
		os << nameComp.name;
	}
	else {
		os << "Entity(Invalid)";
	}
	return os;
}
