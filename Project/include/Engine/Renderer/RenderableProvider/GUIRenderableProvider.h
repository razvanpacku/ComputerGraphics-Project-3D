#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/DataStructures/Transform.h"
class GUIRederableProvider : public IRendarableProvider
{
public:
	MaterialManager::Handle materialHandle;
	TextureManager::TextureHandle textureHandle;
	glm::vec4 uvRect = glm::vec4(0.f, 0.f, 1.f, 1.f); // x, y, width, height in uv space

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	int16_t zOrder = 0; // for layering GUI elements

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if(materialHandle.IsValid() == false)
			return;

		auto& _mm = ResourceManager::Get().meshes;

		Renderable renderable;
		renderable.textureHandle = textureHandle;
		renderable.modelMatrix = modelMatrix;

		renderable.zOrder = zOrder;
		renderable.uvRect = uvRect;
		renderable.layer = RenderLayer::GUI;
		renderable.meshHandle = _mm.GetHandle("primitive/quad");
		renderable.materialHandle = materialHandle;

		out.push_back(renderable);
	}
};