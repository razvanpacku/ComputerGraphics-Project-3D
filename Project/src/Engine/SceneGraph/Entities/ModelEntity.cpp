#include "Engine/SceneGraph/Entities/ModelEntity.h"

#include "Engine/Renderer/RenderableProvider/ModelRenderableProvider.h"

ModelEntity::ModelEntity(const std::string& model, const std::string& name)
	: Entity(name), RenderEntity(name), TransformEntity(name)
	, modelName(model)
{
	renderableProvider = new ModelRenderableProvider();
}

void ModelEntity::SetModel(const std::string& model)
{
	modelName = model;
	renderableComponent->isGenerated = false;
}

void ModelEntity::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
	auto* modelProvider = dynamic_cast<ModelRenderableProvider*>(renderableProvider);
	modelProvider->model = nullptr;
	if (!modelName.empty()) {
		auto& models = ResourceManager::Get().models;
		modelProvider->model = models.Get(modelName);
	}
	modelProvider->modelMatrix = transformComponent->worldMatrix;

	modelProvider->GenerateRenderables(outRenderables);
}

void ModelEntity::UpdateTransform(const glm::mat4& newTransform)
{
	for(auto& renderable : renderableComponent->renderables)
	{
		renderable.modelMatrix = newTransform;
	}
}
