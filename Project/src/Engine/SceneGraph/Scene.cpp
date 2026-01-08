#include "Engine/SceneGraph/Scene.h"

#include "Engine/SceneGraph/Entities/Includes.h"
#include "Engine/SceneGraph/Systems/Includes.h"

#include "Engine/DataStructures/TransformFunctions.h"

#include <stack>
#include <iostream>

// ======================================================
// Scene
// =====================================================

Scene* Scene::activeScene = nullptr;

Scene::Scene()
{
}

Scene::~Scene()
{
    OnDestroy();
	// delete all Entity wrappers
    for (auto& pair : entityMap) {
        delete pair.second;
    }
    entityMap.clear();
	reverseEntityMap.clear();
	registry.clear();
}

void Scene::Init(EngineServices services)
{
    auto* oldSene = GetActiveScene();
    SetActiveScene(this);

    SetupInternalEntities();
    SetupDefaultSystems(services);
    OnCreate();

	// Run startup systems
    for (auto& systemPair : systemOrder) {
        if (systemPair->ShouldRunOnStartup()) {
            systemPair->OnUpdate(0.0f);
        }
    }

    SetActiveScene(oldSene);

	initialized = true;
}

void Scene::Update(double deltaTime)
{
    OnUpdate(deltaTime);
    for (auto& systemPair : systemOrder) {
        systemPair->OnUpdate(deltaTime);
    }
}

void Scene::AddOrMoveEntity(Entity& entity, const Entity* const parent)
{
	if (entity.handle == entt::null) return;
    if (entity.scene != this) return;
	auto parentHandle = parent ? parent->handle : entt::null;
    if(parentHandle == entity.handle || (parent != nullptr && parentHandle == entt::null)) {
        // Cannot set entity as its own parent or to an invalid parent
        return;
	}

    bool isRoot = registry.all_of<RootComponent>(entity.handle);

    if (isRoot) {
        //check that it's the first entity added in the sceneGraph
        if(entityMap.size() > 0) {
            throw std::runtime_error("Root entity must be the first entity added to the scene graph.");
		}
        if (parent != nullptr) {
            throw std::runtime_error("Root entity cannot have a parent.");
        }
		entityMap[entity.handle] = &entity;
		reverseEntityMap[&entity] = entity.handle;
		return;
    }

    // past this point, root is guaranteed to exist
	const Entity* Rparent = parent ? parent : root;
    if (Rparent->scene != this) return;

    if (registry.try_get<InternalNameComponent>(entity.handle)) {
        if (Rparent != root) {
            throw std::runtime_error("Entities with InternalNameComponent must be children of the root entity.");
		}
    }

    auto* childH = registry.try_get<HierarchyComponent>(entity.handle);
    auto& parentH = registry.get<HierarchyComponent>(Rparent->handle);
    if (parentH.parent == entity.handle) {
        // Cannot set parent to a child entity
        return;
    }

	bool isInTree = childH->parent != entt::null;

    // Detach from old parent
    if (isInTree) {
        auto& oldParentH = registry.get<HierarchyComponent>(childH->parent);
        if (oldParentH.firstChild == entity.handle) {
            oldParentH.firstChild = childH->nextSibling;
        }
        if (childH->prevSibling != entt::null)
            registry.get<HierarchyComponent>(childH->prevSibling).nextSibling = childH->nextSibling;
        if (childH->nextSibling != entt::null)
            registry.get<HierarchyComponent>(childH->nextSibling).prevSibling = childH->prevSibling;

		// If it has a TransformComponent, make sure to preserve world transform (this involves possibly updating parent's transform first)
        if (registry.all_of<TransformComponent>(entity.handle)) {
            auto* transformSystem = GetSystem<TransformSystem>();
            if (!transformSystem) {
                throw std::runtime_error("TransformSystem not found in scene.");
			}
			transformSystem->UpdateEntity(entity.handle);
            auto& transformC = registry.get<TransformComponent>(entity.handle);
            glm::mat4 worldMatrix = transformC.worldMatrix;

			glm::mat4 parentWorldMatrix(1.0f);
            auto* newParentTransformC = registry.try_get<TransformComponent>(Rparent->handle);
            if (newParentTransformC) {
                transformSystem->UpdateEntity(Rparent->handle);
                parentWorldMatrix = newParentTransformC->worldMatrix;
            }
            glm::mat4 newLocalMatrix = glm::inverse(parentWorldMatrix) * worldMatrix;
            TransformFunctions::Decompose(transformC, newLocalMatrix);
			transformC.localDirty = false;
        }

		// If it has a UITransformComponent, mark it dirty to recalculate world matrix
        if (registry.all_of<UITransformComponent>(entity.handle)) {
            auto& uiTransformC = registry.get<UITransformComponent>(entity.handle);
            uiTransformC.localDirty = true;
		}
    }

    // Attach to new parent

    childH->parent = Rparent->handle;
    childH->prevSibling = entt::null;
    childH->nextSibling = parentH.firstChild;
    if (parentH.firstChild != entt::null) {
        registry.get<HierarchyComponent>(parentH.firstChild).prevSibling = entity.handle;
    }
    parentH.firstChild = entity.handle;

	entityMap[entity.handle] = &entity;
	reverseEntityMap[&entity] = entity.handle;
}

void Scene::GetChildren(const Entity& entity, std::vector<Entity*>& outChildren)
{
    if (entity.scene != this) return;
    outChildren.clear();
	auto* parentH = registry.try_get<HierarchyComponent>(entity.handle);
    if (!parentH) return;
    entt::entity childHandle = parentH->firstChild;
    while (childHandle != entt::null) {
        auto it = entityMap.find(childHandle);
        if (it != entityMap.end()) {
            outChildren.push_back(it->second);
        }
        auto& childH = registry.get<HierarchyComponent>(childHandle);
        childHandle = childH.nextSibling;
	}
}

void Scene::GetDescendants(const Entity& entity, std::vector<Entity*>& outDescendants)
{
    if (entity.scene != this) return;
    outDescendants.clear();
    auto* parentH = registry.try_get<HierarchyComponent>(entity.handle);
    if (!parentH) return;
    std::vector<entt::entity> stack;
    entt::entity childHandle = parentH->firstChild;
    while (childHandle != entt::null) {
        stack.push_back(childHandle);
        auto& childH = registry.get<HierarchyComponent>(childHandle);
        childHandle = childH.nextSibling;
    }
    while (!stack.empty()) {
        entt::entity currentHandle = stack.back();
        stack.pop_back();
        auto it = entityMap.find(currentHandle);
        if (it != entityMap.end()) {
            outDescendants.push_back(it->second);
        }
        auto& currentH = registry.get<HierarchyComponent>(currentHandle);
        entt::entity childHandle = currentH.firstChild;
        while (childHandle != entt::null) {
            stack.push_back(childHandle);
            auto& childH = registry.get<HierarchyComponent>(childHandle);
            childHandle = childH.nextSibling;
        }
    }
}

Entity* const Scene::FindFirstChild(const std::string& name, const Entity* const parent)
{
	if (parent && parent->scene != this) return nullptr;
	entt::entity parentHandle = parent ? parent->handle : root->handle;
    auto* parentH = registry.try_get<HierarchyComponent>(parentHandle);
    if (!parentH) return nullptr;
    entt::entity childHandle = parentH->firstChild;
    while (childHandle != entt::null) {
        auto it = entityMap.find(childHandle);
        if (it != entityMap.end()) {
            Entity* childEntity = it->second;
            if (childEntity->HasComponent<NameComponent>()) {
                const auto& nameComp = childEntity->GetComponent<NameComponent>();
                if (nameComp.name == name) {
                    return childEntity;
                }
            }
        }
        auto& childH = registry.get<HierarchyComponent>(childHandle);
        childHandle = childH.nextSibling;
    }
	// try again with InternalNameComponent
	childHandle = parentH->firstChild;
    while (childHandle != entt::null) {
        auto it = entityMap.find(childHandle);
        if (it != entityMap.end()) {
            Entity* childEntity = it->second;
            if (childEntity->HasComponent<InternalNameComponent>()) {
				const auto& nameComp = registry.get<InternalNameComponent>(childHandle);
                if (nameComp.name == name) {
                    return childEntity;
                }
            }
        }
        auto& childH = registry.get<HierarchyComponent>(childHandle);
        childHandle = childH.nextSibling;
	}
    return nullptr;
}

Entity* const Scene::FindFirstDescendant(const std::string& name, const Entity* const parent) {
    if (parent && parent->scene != this) return nullptr;
    entt::entity parentHandle = parent ? parent->handle : root->handle;
    auto* parentH = registry.try_get<HierarchyComponent>(parentHandle);
    if (!parentH) return nullptr;

    std::vector<entt::entity> stack;
    entt::entity childHandle = parentH->firstChild;

    // initialize stack with immediate children
    while (childHandle != entt::null) {
        stack.push_back(childHandle);
        auto& childH = registry.get<HierarchyComponent>(childHandle);
        childHandle = childH.nextSibling;
    }

    // depth-first search through descendants
    while (!stack.empty()) {
        entt::entity currentHandle = stack.back();
        stack.pop_back();

        auto it = entityMap.find(currentHandle);
        if (it != entityMap.end()) {
            Entity* currentEntity = it->second;

            // check NameComponent first
            if (currentEntity->HasComponent<NameComponent>()) {
                const auto& nameComp = currentEntity->GetComponent<NameComponent>();
                if (nameComp.name == name) {
                    return currentEntity;
                }
            }

            // check InternalNameComponent next
            if (currentEntity->HasComponent<InternalNameComponent>()) {
				const auto& nameComp = registry.get<InternalNameComponent>(currentHandle);
                if (nameComp.name == name) {
                    return currentEntity;
                }
            }
        }

        // push children of current entity onto the stack
        auto& currentH = registry.get<HierarchyComponent>(currentHandle);
        entt::entity child = currentH.firstChild;
        while (child != entt::null) {
            stack.push_back(child);
            auto& childH = registry.get<HierarchyComponent>(child);
            child = childH.nextSibling;
        }
    }

    return nullptr;
}

Entity* const Scene::FindInternalEntity(const std::string& name) {
    auto view = registry.view<InternalNameComponent>();
    for (auto& ent : view) {
        auto& internalNameComp = view.get<InternalNameComponent>(ent);
        if (internalNameComp.name == name) {
            auto it = entityMap.find(ent);
            if (it != entityMap.end()) {
                return entityMap[ent];
            }
            break;
        }
    }
    return nullptr;
}

void Scene::RemoveEntity(Entity* entity) {
	if (entity == nullptr) return;
	if (entity->scene != this) return;
    if (entity->handle == entt::null) return;
    if(registry.all_of<InternalNameComponent>(entity->handle)) {
        throw std::runtime_error("Cannot remove entity with InternalNameComponent directly.");
	}
    // Remove from parent
    auto* childH = registry.try_get<HierarchyComponent>(entity->handle);
    if (childH && childH->parent != entt::null) {
        auto& parentH = registry.get<HierarchyComponent>(childH->parent);
        if (parentH.firstChild == entity->handle) {
            parentH.firstChild = childH->nextSibling;
        }
        if (childH->prevSibling != entt::null)
            registry.get<HierarchyComponent>(childH->prevSibling).nextSibling = childH->nextSibling;
        if (childH->nextSibling != entt::null)
            registry.get<HierarchyComponent>(childH->nextSibling).prevSibling = childH->prevSibling;
    }
    // Recursively remove children
    std::vector<Entity*> children;
    GetChildren(*entity, children);
    for (Entity* child : children) {
        RemoveEntity(child);
    }
    // Remove from entity map and registry
    entityMap.erase(entity->handle);
	reverseEntityMap.erase(entity);
    delete entity;
    entity = nullptr;
}

const Entity* const Scene::GetRoot() const
{
    return root;
}

Entity* const Scene::GetEntityFromHandle(const entt::entity& handle) const {
    auto it = entityMap.find(handle);
    if (it != entityMap.end()) {
        return it->second;
    }
	return nullptr;
}

void Scene::PrintHierarchy() const {
    if (root == nullptr) return;
    std::stack<std::pair<entt::entity, int>> stack;
    stack.push({ root->handle, 0 });

    while (!stack.empty()) {
        auto [handle, indent] = stack.top();
        stack.pop();

        // Print indentation
        for (int i = 0; i < indent; ++i) std::cout << "  ";

        auto* hier = registry.try_get<HierarchyComponent>(handle);

        auto it = entityMap.find(handle);
        if (it != entityMap.end()) {
            Entity* entity = it->second;
            std::cout << *entity;

			// temp check if it has ui transform component and print its local position, and if it has the dirty tag or the localDirty flag set
			auto* transformC = registry.try_get<UITransformComponent>(handle);
			bool dirty = registry.all_of<UITransformDirtyTag>(handle);
            if (transformC) {
				std::cout << " [UITransform pos: (" << transformC->position.x << ", " << transformC->position.y << ")" 
					<< (dirty || transformC->localDirty ? ", dirty" : "") << "]";
            }

            if (hier && hier->firstChild != entt::null) {
                std::cout << ":";
            }
        }

        std::cout << "\n";

        if (hier) {
            std::vector<entt::entity> children;
            entt::entity child = hier->firstChild;
            while (child != entt::null) {
                children.push_back(child);
                child = registry.get<HierarchyComponent>(child).nextSibling;
            }
            for (auto it = children.begin(); it != children.end(); ++it) {
                stack.push({ *it, indent + 1 });
            }
        }
    }
}

void Scene::MakeInternal(Entity* entity, const std::string& name) {
    if (!entity) {
        throw std::runtime_error("Entity is null");
    }

    // check that no other entity has the same InternalNameComponent name
    auto view = registry.view<InternalNameComponent>();
    for (auto ent : view) {
        auto& internalNameComp = view.get<InternalNameComponent>(ent);
        if (internalNameComp.name == name) {
            throw std::runtime_error("An entity with the same InternalNameComponent name already exists: " + name);
        }
    }

    registry.emplace<InternalNameComponent>(entity->handle, name);
}

void Scene::SetupInternalEntities()
{
	root = CreateInternalEntity<Root>("Root", InternalComp<RootComponent>());
    CreateInternalEntity<Camera>("Camera");
	CreateInternalEntity<Light>("Light");
}

void Scene::SetupDefaultSystems(EngineServices services)
{
	AddSystem<PhysicsSystem>(10, &registry);
	AddSystem<CollisionSystem>(20, &registry);
	AddSystem<TransformSystem>(50, &registry);
    AddSystem<UITransformSystem>(51, &registry);
    AddSystem<RenderSystem>(100, services.renderer, &registry);
}
