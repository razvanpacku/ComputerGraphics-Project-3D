#include "Engine/SceneGraph/Entities/RenderEntity.h"

#include "Engine/Renderer/RenderableProvider/IRendarableProvider.h"

// ================================================================
// RenderEntity
// ================================================================

RenderEntity::RenderEntity(const std::string& name)
	: Entity(name)
{
	renderableComponent = &AddComponent<RenderableComponent>();
	renderableComponent->RenderableGenerator = 
		[this](std::vector<Renderable>& outRenderables) {
			this->ProvideRenderables(outRenderables);
		};
	renderableComponent->RenderableUpdater = 
		[this](double deltaTime, std::vector<Renderable>& renderables) {
			this->UpdateRenderables(deltaTime, renderables);
		};

	renderableComponent->OnTransformUpdated = 
		[this](const glm::mat4& newTransform) {
			this->UpdateTransform(newTransform);
		};
}

RenderEntity::~RenderEntity()
{
	delete renderableProvider;
}
