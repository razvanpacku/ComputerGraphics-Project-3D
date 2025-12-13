#include "Engine/Renderer/RenderQueue.h"

#include <algorithm>

// =================================================
// RenderQueue
// =================================================

void RenderQueue::Clear()
{
	opaque.clear();
	transparent.clear();
	gui.clear();
	shadowCasters.clear();
	nextSubmitIndex = 1;
}

void RenderQueue::Push(const Renderable& renderable)
{

	RenderSubmission submission;
	submission.item = renderable;
	submission.sortKey = renderable.GetSortKey();

	if(submission.item.castShadows) {
		shadowCasters.push_back(submission);
	}

	// Check if renderable is within the view frustum if it has bounds
	if (renderable.hasBounds) {
		//get transformed AABB
		glm::mat4 modelMatrix = renderable.transform.GetModelMatrix();
		BoundingBox transformedAABB = TransformAABB(renderable.aabb, modelMatrix);

		if (!AABBInFrustum(viewFrustum, transformedAABB)) {
			// Cull the renderable
			return;
		}
	}

	switch (renderable.layer) {
		case RenderLayer::Opaque:
			opaque.push_back(std::move(submission));
			break;
		case RenderLayer::Transparent:
			transparent.push_back(std::move(submission));
			break;
		case RenderLayer::GUI:
			gui.push_back(std::move(submission));
			break;
	}
}

void RenderQueue::Push(const std::vector<Renderable>& renderables)
{
	for (const auto& renderable : renderables) {
		Push(renderable);
	}
}

std::vector<RenderSubmission>& RenderQueue::GetSortedLayer(RenderLayer layer)
{
	std::vector<RenderSubmission>* target = nullptr;

	switch (layer) {
	case RenderLayer::Opaque:
		target = &opaque;
		break;
	case RenderLayer::Transparent:
		target = &transparent;
		break;
	case RenderLayer::GUI:
		target = &gui;
		break;
	}

	std::sort(target->begin(), target->end());
	return *target;
}

std::vector<RenderSubmission>& RenderQueue::GetShadowCasters()
{
	std::sort(shadowCasters.begin(), shadowCasters.end());
	return shadowCasters;
}

size_t RenderQueue::TotalSize() const
{
	return opaque.size() + transparent.size() + gui.size();
}