#pragma once
#include "../Renderable.h"

class IRendarableProvider
{
public:
	virtual void GenerateRenderables(std::vector<Renderable>& out) = 0;
	virtual ~IRendarableProvider() = default;
};

