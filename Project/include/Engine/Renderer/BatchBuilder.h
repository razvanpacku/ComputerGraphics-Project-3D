#pragma once
#include "Renderable.h"

#include <vector>

class BatchBuilder
{
public:
	// trashes sorted vector
	// this shouldn't matter, as the RenderQueue shouldn't need the old unbatched data
	static std::vector<RenderSubmission> Build(std::vector<RenderSubmission>& sorted);
private:
	static void InitInstanceData(RenderSubmission& cmd);
	static void AppendInstanceData(RenderSubmission& batch, const Renderable& r);
};

