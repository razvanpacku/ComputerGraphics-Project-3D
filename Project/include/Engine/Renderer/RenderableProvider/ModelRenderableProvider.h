#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/Components/Transform.h"

class ModelRenderableProvider : public IRendarableProvider
{
public:
	Model* model = nullptr;
	Transform transform;

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if (!model) return;

		auto& _mm = ResourceManager::Get().meshes;

		for (const auto& meshEntry : model->meshEntries)
		{
			Renderable renderable;
			renderable.meshHandle = meshEntry.mesh;
			renderable.materialHandle = meshEntry.material;
			renderable.transform = transform;

			Mesh* meshPtr = _mm.Get(meshEntry.mesh);
			if (meshPtr)
			{
				renderable.aabb = meshPtr->boundingBox;
				renderable.hasBounds = true;
			}

			out.push_back(renderable);
		}
	}
};