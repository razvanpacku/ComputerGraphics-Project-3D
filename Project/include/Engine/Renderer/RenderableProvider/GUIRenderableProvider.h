#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/Components/Transform.h"
class GUIRederableProvider : public IRendarableProvider
{
public:
	MaterialManager::Handle materialHandle;
	TextureManager::TextureHandle textureHandle;
	Transform transform;
	glm::vec2 relativePosition = glm::vec2(0.5f, 0.5f); // relative position on screen (0..1)
	glm::vec2 relativeSize = glm::vec2(1.f, 1.f); // relative size to screen (0..1)
	glm::vec2 anchorPoint = glm::vec2(0.5f, 0.5f); // anchor point for positioning (0..1)
	glm::vec4 uvRect = glm::vec4(0.f, 0.f, 1.f, 1.f); // x, y, width, height in uv space
	int16_t zOrder = 0; // for layering GUI elements

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		if(materialHandle.IsValid() == false)
			return;

		auto& _mm = ResourceManager::Get().meshes;

		Renderable renderable;
		renderable.textureHandle = textureHandle;
		renderable.transform = transform;

		renderable.relativePosition = relativePosition;
		renderable.relativeSize = relativeSize;
		renderable.anchorPoint = anchorPoint;

		renderable.zOrder = zOrder;
		renderable.uvRect = uvRect;
		renderable.layer = RenderLayer::GUI;
		renderable.meshHandle = _mm.GetHandle("primitive/quad");
		renderable.materialHandle = materialHandle;

		out.push_back(renderable);
	}
};