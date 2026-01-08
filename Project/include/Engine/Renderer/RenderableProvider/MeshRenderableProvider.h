#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/DataStructures/Transform.h"

class MeshRenderableProvider : public IRendarableProvider
{
public:
	MeshManager::Handle meshHandle = {};
	Mesh* dynamicMesh = nullptr; // optional direct mesh pointer for dynamic meshes
	MaterialManager::Handle materialHandle = {};
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	bool castShadows = false;

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if ((!meshHandle.IsValid() && !dynamicMesh) || !materialHandle.IsValid()) return;

		auto& _mm = ResourceManager::Get().meshes;

		Renderable renderable;
		renderable.meshHandle = meshHandle;
		renderable.mesh = dynamicMesh;
		renderable.materialHandle = materialHandle;
		renderable.modelMatrix = modelMatrix;
		renderable.castShadows = castShadows;

		Mesh* meshPtr = dynamicMesh ? dynamicMesh : _mm.Get(meshHandle);
		if (meshPtr)
		{
			renderable.aabb = meshPtr->boundingBox;
			renderable.hasBounds = true;
			renderable.cullBackfaces = meshPtr->cullBackfaces;
		}

		out.push_back(renderable);
	}
}; 
