#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ModelManager.h"
#include "Engine/Components/Transform.h"

class ModelRenderableProvider : public IRendarableProvider
{
public:
	Model* model = nullptr;
	Transform transform;

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if (!model) return;;
		for (const auto& meshEntry : model->meshEntries)
		{
			Renderable renderable;
			renderable.meshHandle = meshEntry.mesh;
			renderable.materialHandle = meshEntry.material;
			renderable.transform = transform;

			out.push_back(renderable);
		}
	}
};