#pragma once
#include <entt/entt.hpp>
#include "Engine/Components/Components.h"

//#include "Scene.h"

//forward declaration
class Scene;

// ======================================================
// Entity
//
// Represents an entity in the scene graph.
// ======================================================
class Entity
{
public:
	Entity(const std::string& name = "");
	virtual ~Entity();

	// --- Component Management ---
	template<typename T, typename... Args>
	decltype(auto) AddComponent(Args&&... args);
	template<typename T>
	T& GetComponent();
	template<typename T>
	const T& GetComponent() const;
	template<typename T>
	bool HasComponent() const;
	template<typename T>
	void RemoveComponent();

	// --- Entity tree wrappers ---
	Entity* const GetParent() const;
	void SetParent(Entity* const parent = nullptr);
	std::vector<Entity*> GetChildren() const;
	std::vector<Entity*> GetDescendants() const;
	Entity* const FindFirstChild(const std::string& name) const;
	Entity* const FindFirstDescendant(const std::string& name) const;
	void Destroy();

private:
	entt::entity handle;
	Scene* scene;

	static uint64_t nextID;

	friend class Scene;
	friend std::ostream& operator<<(std::ostream& os, const Entity& entity);
};

// A helper struct to hold arguments for entities.
template<typename... Args>
struct EntityCtor {
	std::tuple<Args...> args;

	explicit EntityCtor(Args&&... a)
		: args(std::forward<Args>(a)...)
	{
	}
};

// --- Template Implementations ---
#include "Scene.h"

template<typename T, typename... Args>
decltype(auto) Entity::AddComponent(Args&&... args)
{
	if constexpr (InternalComponentType<T>) {
		throw std::runtime_error("Cannot add InternalComponent directly");
	}
	if constexpr (std::is_empty_v<T>) {
		scene->registry.emplace<T>(handle, std::forward<Args>(args)...);
		return;
	} 
	else {
		return scene->registry.emplace<T>(handle, std::forward<Args>(args)...);
	}
}

template<typename T>
T& Entity::GetComponent()
{
	if constexpr (ReadOnlyComponentType<T>) {
		throw std::runtime_error("Cannot get Read-Only Component for modification");
	}
	return scene->registry.get<T>(handle);
}

template<typename T>
const T& Entity::GetComponent() const {
	return scene->registry.get<T>(handle);
}

template<typename T>
bool Entity::HasComponent() const
{
	return scene->registry.all_of<T>(handle);
}

template<typename T>
void Entity::RemoveComponent()
{
	if constexpr (InternalComponentType<T>) {
		throw std::runtime_error("Cannot add InternalComponent directly");
	}
	scene->registry.remove<T>(handle);
}

