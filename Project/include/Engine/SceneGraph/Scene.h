#pragma once
#include <entt/entt.hpp>
#include "Engine/Components/Components.h"

#include <typeindex>

//forward declaration
class Entity;
class Root;
class ISystem;
class Renderer;

struct EngineServices {
	Renderer* renderer = nullptr;
};

// ======================================================
// Scene
// 
// Represents a scene containing entities and components, and update logic.
// The user should derive from this class to create specific scenes, possibly using their own derived Entity classes or components.
// ======================================================
class Scene
{
public:

	Scene();
	virtual ~Scene();
	void Init(EngineServices services);

	void Update(double deltaTime);

	// --- Scene Interface ---
	virtual void OnCreate() {}
	virtual void OnUpdate(double deltaTime) {}
protected:
	virtual void OnDestroy() {}

	// --- Entity Management ---
public:

	void AddOrMoveEntity(Entity& entity, const Entity* const parent = nullptr);
	void GetChildren(const Entity& entity, std::vector<Entity*>& outChildren);
	void GetDescendants(const Entity& entity, std::vector<Entity*>& outDescendants);
	Entity* const FindFirstChild(const std::string& name, const Entity* const parent = nullptr);
	Entity* const FindFirstDescendant(const std::string& name, const Entity* const parent = nullptr);
	Entity* const FindInternalEntity(const std::string& name);
	void RemoveEntity(Entity* entity);

	const Entity* const GetRoot() const;

	// --- System Management ---
protected:
	//void AddSystem(ISystem* system);
	template<typename T, typename... Args>
	T& AddSystem(int16_t order, Args&&... args);
	template<typename T>
	T* const GetSystem() const;

	// --- Static Active Scene Management ---
public:
	static Scene* GetActiveScene() { return activeScene; }
	static void SetActiveScene(Scene* scene) { activeScene = scene; }
	
	// --- Debug functions ---
	void PrintHierarchy() const;
private:
	static Scene* activeScene;
	entt::registry registry;

	Root* root;

	std::unordered_map<entt::entity, Entity*> entityMap;

	std::unordered_map<std::type_index, ISystem*> systems;
	std::vector<ISystem*> systemOrder;

	void MakeInternal(Entity* entity, const std::string& name);

	template<InternalComponentType T, typename... Args>
	decltype(auto) AddInternalComponent(Entity* entity, Args&&... args);

	template<typename EntityT, typename EntityCtorT, typename... InternalComponents>
	EntityT* CreateInternalEntity(EntityCtorT&& ctor, const std::string& name, InternalComponents&&... components);
	template<typename EntityT, typename... InternalComponents>
	EntityT* CreateInternalEntity(const std::string& name, InternalComponents&&... components);


	void SetupInternalEntities();
	void SetupDefaultSystems(EngineServices services);

	friend class Entity;
};

#include "Entity.h"
#include "ISystem.h"

template<typename T, typename... Args>
T& Scene::AddSystem(int16_t order, Args&&... args)
{
	static_assert(std::is_base_of_v<ISystem, T>,
		"T must derive from ISystem");

	std::type_index type = typeid(T);

	if (systems.contains(type))
	{
		throw std::runtime_error(
			std::string("System already exists: ") + typeid(T).name()
		);
	}

	auto* system = new T(this, order, std::forward<Args>(args)...);
	systems[type] = system;
	systemOrder.push_back(system);

	// Sort systems based on order
	std::sort(systemOrder.begin(), systemOrder.end(),
		[](ISystem* a, ISystem* b) {
			return a->GetOrder() < b->GetOrder();
		}
	);

	return *system;
}

template<typename T>
T* const Scene::GetSystem() const
{
	static_assert(std::is_base_of_v<ISystem, T>,
		"T must derive from ISystem");
	std::type_index type = typeid(T);
	auto it = systems.find(type);
	if (it != systems.end())
	{
		return dynamic_cast<T*>(it->second);
	}
	return nullptr;
}

// Add an internal component to an entity, forwarding constructor arguments.
template<InternalComponentType T, typename... Args>
decltype(auto) Scene::AddInternalComponent(Entity* entity, Args&&... args)
{
	if (!entity) {
		throw std::runtime_error("Entity is null");
	}
	if constexpr (std::is_empty_v<T>) {
		registry.emplace<T>(entity->handle, std::forward<Args>(args)...);
		return; // returns void
	}
	else {
		return registry.emplace<T>(entity->handle, std::forward<Args>(args)...);
	}
}

// Create an internal entity of type EntityT with specified constructor arguments and internal components and their arguments.
template<typename EntityT, typename EntityCtorT, typename... InternalComponents>
EntityT* Scene::CreateInternalEntity(
	EntityCtorT&& ctor,
	const std::string& name,
	InternalComponents&&... components
)
{
	static_assert(std::is_base_of_v<Entity, EntityT>, "EntityT must derive from Entity");

	// Construct entity with forwarded ctor args
	EntityT* entity = std::apply(
		[](auto&&... args) {
			return new EntityT(std::forward<decltype(args)>(args)...);
		},
		ctor.args
	);

	MakeInternal(entity, name);

	(
		std::apply(
			[&](auto&&... args) {
				AddInternalComponent<
					typename std::decay_t<InternalComponents>::component_type
				>(entity, std::forward<decltype(args)>(args)...);
			},
			components.args
		),
		...
	);

	AddOrMoveEntity(*entity);

	return entity;
}

// Overload of the above function for default constructor of EntityT.
template<typename EntityT, typename... InternalComponents>
EntityT* Scene::CreateInternalEntity(
	const std::string& name,
	InternalComponents&&... components
)
{
	return CreateInternalEntity<EntityT>(
		EntityCtor<>(),
		name,
		std::forward<InternalComponents>(components)...
	);
}