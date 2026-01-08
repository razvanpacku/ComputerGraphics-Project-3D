#pragma once
#include "Engine/Renderer/Renderable.h"
#include <functional>
#include <glm/glm.hpp>

struct RenderableComponent
{
	std::vector<Renderable> renderables;
	std::function<void(std::vector<Renderable>&)> RenderableGenerator;
	std::function<void(double deltaTime, std::vector<Renderable>&)> RenderableUpdater;
	std::function<void(const glm::mat4&)> OnTransformUpdated;

	bool isGenerated = false;

	const std::vector<Renderable>& GetRenderables()
	{
		if (RenderableGenerator && !isGenerated)
		{
			renderables.clear();
			RenderableGenerator(renderables);
			isGenerated = true;
		}
		return renderables;
	}

	void UpdateRenderables(double deltaTime)
	{
		if (RenderableUpdater)
		{
			RenderableUpdater(deltaTime, renderables);
		}
	}

	void UpdateTransform(const glm::mat4& newTransform)
	{
		if (OnTransformUpdated)
		{
			OnTransformUpdated(newTransform);
		}
	}

	void UpdateZOrder(uint16_t newZOrder) {
		for (auto& renderable : renderables) {
			renderable.zOrder = newZOrder;
		}
	}
};
