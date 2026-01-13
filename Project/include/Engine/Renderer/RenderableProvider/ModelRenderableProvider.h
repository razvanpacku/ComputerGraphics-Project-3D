#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/DataStructures/Transform.h"

class ModelRenderableProvider : public IRendarableProvider
{
public:
	Model* model = nullptr;
	glm::mat4 modelMatrix = glm::mat4(1.0f);

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
			renderable.modelMatrix = modelMatrix;

			bool isTransparent = false;

			Mesh* meshPtr = _mm.Get(meshEntry.mesh);
			if (meshPtr)
			{
				renderable.aabb = meshPtr->boundingBox;
				renderable.hasBounds = true;

				//check material to see if renderable should cast/receive shadows and if it's transparent
				auto* material = _mam.Get(meshEntry.material);
				if(material){
					renderable.castShadows = material->castShadows;
					auto val = material->GetUniform<int>("receiveShadows");
					renderable.receiveShadows = val.value_or(false);

					auto transVal = material->GetUniform<int>("isTransparent");
					isTransparent = transVal.value_or(0) != 0;
				}
			}

			if(isTransparent)
			{
				renderable.layer = RenderLayer::Transparent;
			}

			out.push_back(renderable);
		}
	}
};