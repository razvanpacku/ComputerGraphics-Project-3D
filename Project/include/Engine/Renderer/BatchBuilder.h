#pragma once
#include "Renderable.h"

#include <vector>

class BatchBuilder
{
public:
	static std::vector<RenderSubmission> Build(const std::vector<RenderSubmission>& sorted);
private:
	static void InitInstanceData(RenderSubmission& cmd);
	static void AppendInstanceData(RenderSubmission& batch, const Renderable& r);
};

