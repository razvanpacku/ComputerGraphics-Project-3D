#include "Engine/Renderer/RenderQueue.h"

// =================================================
// RenderQueue
// =================================================

void RenderQueue::Clear()
{
	opaque = std::priority_queue<RenderSubmission>();
	transparent = std::priority_queue<RenderSubmission>();
	gui = std::priority_queue<RenderSubmission>();
	nextSubmitIndex = 1;
}

void RenderQueue::Push(const Renderable& renderable)
{
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

	RenderSubmission submission;
	submission.item = renderable;
	submission.sortKey = renderable.GetSortKey();

	switch (renderable.layer) {
		case RenderLayer::Opaque:
			opaque.push(std::move(submission));
			break;
		case RenderLayer::Transparent:
			transparent.push(std::move(submission));
			break;
		case RenderLayer::GUI:
			gui.push(std::move(submission));
			break;
	}
}

void RenderQueue::Push(const std::vector<Renderable>& renderables)
{
	for (const auto& renderable : renderables) {
		Push(renderable);
	}
}

std::vector<RenderSubmission>& RenderQueue::GetSortedLayer(RenderLayer layer) const
{
	static std::vector<RenderSubmission> sortedSubmissions;
	sortedSubmissions.clear();
	std::priority_queue<RenderSubmission> tempQueue;
	switch (layer) {
		case RenderLayer::Opaque:
			tempQueue = opaque;
			break;
		case RenderLayer::Transparent:
			tempQueue = transparent;
			break;
		case RenderLayer::GUI:
			tempQueue = gui;
			break;
	}
	while (!tempQueue.empty()) {
		sortedSubmissions.push_back(tempQueue.top());
		tempQueue.pop();
	}
	return sortedSubmissions;
}

size_t RenderQueue::TotalSize() const
{
	return opaque.size() + transparent.size() + gui.size();
}