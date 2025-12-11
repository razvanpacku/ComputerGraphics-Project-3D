#pragma once
#include "Renderable.h"

#include <vector>
#include <queue>

// =================================================
// RenderQueue
//
// Collects and sorts RenderSubmissions for rendering per frame.
// =================================================
class RenderQueue
{
public:
	void Clear();

	// Add a renderable to the queue (copy is stoed to allow temporary objects)
	void Push(const Renderable& renderable);
	void Push(const std::vector<Renderable>& renderables);

	// Get a sorted list of submissions for a specific layer, called by the Renderer
	std::vector<RenderSubmission>& GetSortedLayer(RenderLayer layer) const;

	size_t TotalSize() const;
private:
	std::priority_queue<RenderSubmission> opaque;
	std::priority_queue<RenderSubmission> transparent;
	std::priority_queue<RenderSubmission> gui;

	uint64_t nextSubmitIndex = 1;

};

