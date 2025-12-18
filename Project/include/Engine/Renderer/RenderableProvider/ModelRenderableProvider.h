#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/DataStructures/Transform.h"

class ModelRenderableProvider : public IRendarableProvider
{
public:
	Model* model = nullptr;
	Transform transform;

	bool isTransparent = false;
	glm::vec3 cameraPosition = glm::vec3(0.0f);

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

			if(isTransparent)
			{
				// calculate distance from camera for sorting
				glm::vec3 renderablePos = renderable.transform.position;
				float distance = glm::length(cameraPosition - renderablePos);
				renderable.sortDistance = distance;
				renderable.layer = RenderLayer::Transparent;
			}

			out.push_back(renderable);
		}
	}
};