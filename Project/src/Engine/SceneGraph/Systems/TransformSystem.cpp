#include "Engine/SceneGraph/Systems/TransformSystem.h"

#include "Engine/Components/Components.h"
#include "Engine/DataStructures/TransformFunctions.h"

// ======================================================
// TransformSystem
// ======================================================

TransformSystem::TransformSystem(Scene* scene, int16_t order, entt::registry* registry)
	: ISystem(scene, order), registry(registry)
{
	runOnStartup = true;
}

void TransformSystem::OnUpdate(double deltaTime)
{
	// Get the root entities with TransformDirtyTag
	// We do this by checking for an entity with TransformDirtyTag if their parent is null or does not have TransformDirtyTag
	auto view = registry->view<TransformDirtyTag, TransformComponent>();
	for(auto entity : view)
	{
		auto* entH = registry->try_get<HierarchyComponent>(entity);
		bool isRootDirty =
			!entH ||
			entH->parent == entt::null ||
			!registry->all_of<TransformDirtyTag, TransformComponent>(entH->parent);
		if(isRootDirty)
		{
			// this is a root dirty entity, start updating from here
			UpdateSubtree(entity);
		}
	}
}

bool TransformSystem::MarkDirty(entt::entity entity)
{
	auto* transformC = registry->try_get<TransformComponent>(entity);
	if(!transformC)
	{
		return false;
	}
	bool dirty = registry->all_of<TransformDirtyTag>(entity);

	if (dirty) {
		// already dirty, no need to do anything
		return true;
	}

	registry->emplace<TransformDirtyTag>(entity);

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

void TransformSystem::UpdateTransform(entt::entity entity)
{
	auto* transformC = registry->try_get<TransformComponent>(entity);
	if (transformC) {
		// Update local matrix
		if(transformC->localDirty) {
			// If localDirty is true, we need to recompute local matrix
			// It can be false if only the ancestor's local/global matrix changed
			transformC->localMatrix = TransformFunctions::ComputeLocal(transformC->position, transformC->rotation, transformC->scale);
			transformC->localDirty = false;
		}

		// Update world matrix
		auto* hierC = registry->try_get<HierarchyComponent>(entity);
		if (hierC && hierC->parent != entt::null) {
			auto* parentTransformC = registry->try_get<TransformComponent>(hierC->parent);
			if (parentTransformC) {
				transformC->worldMatrix = TransformFunctions::ComputeGlobal(parentTransformC->worldMatrix, transformC->localMatrix);
			}
			else {
				transformC->worldMatrix = transformC->localMatrix;
			}
		}
		else {
			transformC->worldMatrix = transformC->localMatrix;
		}


		// Remove dirty tag
		registry->remove<TransformDirtyTag>(entity);
	}
}

void TransformSystem::UpdateEntity(entt::entity entity)
{
	// check if entity is dirty
	if (!registry->all_of<TransformDirtyTag, TransformComponent>(entity)) {
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
			!registry->all_of<TransformDirtyTag, TransformComponent>(hierC->parent))
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

void TransformSystem::UpdateSubtree(entt::entity entity)
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