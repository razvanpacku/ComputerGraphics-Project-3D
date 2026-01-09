#pragma once
#include "../ISystem.h"
#include <entt/entt.hpp>

template<typename TransformComponentType, typename DirtyTagType>
class TransformSystemBase : public ISystem
{
public:
	TransformSystemBase(Scene* scene, int16_t order = 0, entt::registry* registry = nullptr);
	~TransformSystemBase() override = default;
	virtual void OnUpdate(double deltaTime) override;

	bool MarkDirty(entt::entity entity);

	virtual void UpdateTransform(entt::entity entity) = 0;
	void UpdateEntity(entt::entity entity);

	virtual std::string GetName() const = 0;
protected:
	entt::registry* registry = nullptr;

	double deltaTime = 0.0;

	void UpdateSubtree(entt::entity entity);
};

template<typename TransformComponentType, typename DirtyTagType>
TransformSystemBase<TransformComponentType, DirtyTagType>::TransformSystemBase(Scene* scene, int16_t order, entt::registry* registry)
	: ISystem(scene, order), registry(registry)
{
	runOnStartup = true;
}

template<typename TransformComponentType, typename DirtyTagType>
void TransformSystemBase<TransformComponentType, DirtyTagType>::OnUpdate(double deltaTime)
{
	this->deltaTime = deltaTime;
	// Get the root entities with TransformDirtyTag
	// We do this by checking for an entity with TransformDirtyTag if their parent is null or does not have TransformDirtyTag
	auto view = registry->view<DirtyTagType>();

	std::vector<entt::entity> dirtyList;
	dirtyList.reserve(view.size());

	for (auto entity : view) {
		dirtyList.push_back(entity);
	}

	for (auto entity : dirtyList)
	{
		auto* entH = registry->try_get<HierarchyComponent>(entity);
		bool isRootDirty =
			!entH ||
			entH->parent == entt::null ||
			!registry->all_of<DirtyTagType>(entH->parent);
		if (isRootDirty)
		{
			// this is a root dirty entity, start updating from here
			UpdateSubtree(entity);
		}
	}
}

template<typename TransformComponentType, typename DirtyTagType>
bool TransformSystemBase<TransformComponentType, DirtyTagType>::MarkDirty(entt::entity entity)
{
	auto* transformC = registry->try_get<TransformComponentType>(entity);
	if (!transformC)
	{
		return false;
	}
	bool dirty = registry->all_of<DirtyTagType>(entity);

	if (dirty) {
		// already dirty, no need to do anything
		return true;
	}

	registry->emplace<DirtyTagType>(entity);

	auto& hierarchyC = registry->get<HierarchyComponent>(entity);
	// traverse descendants recursively and mark them dirty
	// children with no TransformComponent or that are already dirty are skipped by the code above
	entt::entity child = hierarchyC.firstChild;
	while (child != entt::null) {
		MarkDirty(child);
		auto& childH = registry->get<HierarchyComponent>(child);
		child = childH.nextSibling;
	}
	return true;
}

template<typename TransformComponentType, typename DirtyTagType>
void TransformSystemBase<TransformComponentType, DirtyTagType>::UpdateEntity(entt::entity entity)
{
	// check if entity is dirty
	if (!registry->all_of<DirtyTagType>(entity)) {
		// not dirty, nothing to do
		return;
	}

	std::vector <entt::entity> path;

	// first, get the root of the subtree that contains this entity by traversing up the hierarchy
	entt::entity current = entity;
	path.push_back(current);
	while (true) {
		auto* hierC = registry->try_get<HierarchyComponent>(current);
		// check if parent is dirty
		if (!hierC || hierC->parent == entt::null ||
			!registry->all_of<DirtyTagType>(hierC->parent))
		{
			break;
		}
		path.push_back(hierC->parent);
		current = hierC->parent;
	}

	// For efficiency, we only update the path from the root to the entity, splitting the original subtree into smaller parts
	// If another child of the root needs to be updated later, its parent will have already been updated, so we can just update it directly
	for (auto it = path.rbegin(); it != path.rend(); ++it) {
		UpdateTransform(*it);
	}
}

template<typename TransformComponentType, typename DirtyTagType>
void TransformSystemBase<TransformComponentType, DirtyTagType>::UpdateSubtree(entt::entity entity)
{
	// Done iteratively using a stack
	std::vector<entt::entity> stack;
	stack.push_back(entity);

	while (!stack.empty()) {
		// Update parents first
		entt::entity current = stack.back();
		stack.pop_back();

		UpdateTransform(current);

		// Push children to stack
		auto* hier = registry->try_get<HierarchyComponent>(current);
		if (hier) {
			entt::entity child = hier->firstChild;
			while (child != entt::null) {
				stack.push_back(child);
				child = registry->get<HierarchyComponent>(child).nextSibling;
			}
		}
	}
}



