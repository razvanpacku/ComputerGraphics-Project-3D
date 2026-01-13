#pragma once
#include "../Entity.h"

#include "Engine/Components/RenderableComponent.h"

//forward declarations
class IRendarableProvider;

// ================================================================
// RenderEntity
// 
// An entity that can be rendered.
// ================================================================
class RenderEntity : public virtual Entity
{
public:
	RenderEntity(const std::string& name);
	virtual ~RenderEntity();

protected:
	RenderableComponent& GetRenderableComponent() { return *renderableComponent; }

	virtual void ProvideRenderables(std::vector<Renderable>& outRenderables) = 0;
	virtual void UpdateRenderables(double deltaTime, std::vector<Renderable>& renderables) {};
	virtual void UpdateTransform(const glm::mat4& newTransform) {};

	RenderableComponent* renderableComponent;
	IRendarableProvider* renderableProvider = nullptr;
};

