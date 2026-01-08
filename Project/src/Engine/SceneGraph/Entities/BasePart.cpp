#include "Engine/SceneGraph/Entities/BasePart.h"

#include "Engine/Renderer/RenderableProvider/MeshRenderableProvider.h"

std::string BasePartToModel(const BasePartShape& shape)
{
	switch (shape)
	{
	case BasePartShape::CUBE:
		return "primitive/cube";
	case BasePartShape::SPHERE:
		return "primitive/uv_sphere";
	case BasePartShape::QUAD:
		return "primitive/quad";
	case BasePartShape::QUAD_SINGLE_FACE:
		return "primitive/quad_backface";
	case BasePartShape::QUAD_DOUBLE_SIDED:
		return "primitive/quad_doubleSided";
	default:
		return "";
	}
}

// ================================================================
// BasePart
// ================================================================

BasePart::BasePart(const BasePartShape& shape, const std::string& material, const std::string& name)
	: Entity(name), RenderEntity(name), TransformEntity(name), shape(shape), materialName(material)
{
	renderableProvider = new MeshRenderableProvider();
}

void BasePart::SetShape(BasePartShape newShape)
{
	shape = newShape;
	renderableComponent->isGenerated = false;
}

void BasePart::SetMaterial(const std::string& newMaterial)
{
	materialName = newMaterial;
	renderableComponent->isGenerated = false;
}

void BasePart::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
	auto* meshProvider = dynamic_cast<MeshRenderableProvider*>(renderableProvider);
	
	meshProvider->meshHandle = ResourceManager::Get().meshes.GetHandle(BasePartToModel(shape));
	meshProvider->materialHandle = ResourceManager::Get().materials.GetHandle(materialName);

	meshProvider->modelMatrix = transformComponent->worldMatrix;

	if (shape != BasePartShape::QUAD && shape != BasePartShape::QUAD_SINGLE_FACE)
		meshProvider->castShadows = true;

	meshProvider->GenerateRenderables(outRenderables);
}

void BasePart::UpdateTransform(const glm::mat4& newTransform)
{
	for (auto& renderable : renderableComponent->renderables)
	{
		renderable.modelMatrix = newTransform;
	}
}