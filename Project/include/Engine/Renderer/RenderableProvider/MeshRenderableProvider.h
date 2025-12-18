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
	Transform transform;

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if ((!meshHandle.IsValid() && !dynamicMesh) || !materialHandle.IsValid()) return;

		auto& _mm = ResourceManager::Get().meshes;

		Renderable renderable;
		renderable.meshHandle = meshHandle;
		renderable.mesh = dynamicMesh;
		renderable.materialHandle = materialHandle;
		renderable.transform = transform;

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
