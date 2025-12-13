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
		auto& _mam = ResourceManager::Get().materials;

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

				//check material to see if renderable should cast/receive shadows
				auto* material = _mam.Get(meshEntry.material);
				if(material){
					renderable.castShadows = material->castShadows;
					auto val = material->GetUniform<int>("receiveShadows");
					renderable.receiveShadows = val.value_or(false);
				}
			}

			out.push_back(renderable);
		}
	}
};