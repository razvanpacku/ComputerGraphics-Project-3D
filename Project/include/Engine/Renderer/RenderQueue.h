#pragma once
#include "Renderable.h"
#include "Culling/Frustum.h"

#include <vector>

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
	std::vector<RenderSubmission>& GetSortedLayer(RenderLayer layer);

	std::vector<RenderSubmission>& GetShadowCasters();

	size_t TotalSize() const;

	void SetViewFrustum(const Frustum& frustum) { viewFrustum = frustum; }
private:
	std::vector<RenderSubmission> opaque;
	std::vector<RenderSubmission> transparent;
	std::vector<RenderSubmission> gui;
	
	std::vector<RenderSubmission> shadowCasters;

	uint64_t nextSubmitIndex = 1;

	Frustum viewFrustum;

};

