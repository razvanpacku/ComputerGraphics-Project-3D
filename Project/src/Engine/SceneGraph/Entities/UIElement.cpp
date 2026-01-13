#include "Engine/SceneGraph/Entities/UIElement.h"

#include "Engine/Renderer/RenderableProvider/GUIRenderableProvider.h"

// ================================================================
// UIElement
// ===============================================================

UIElement::UIElement(const std::string& texture, const glm::vec4& sprite, const std::string& name)
	: Entity(name), RenderEntity(name), UITransformEntity(name), textureName(texture), spriteSize(sprite)
{
	renderableProvider = new GUIRederableProvider();
}

void UIElement::SetTexture(const std::string& texture)
{
	textureName = texture;
	renderableComponent->isGenerated = false;
}

void UIElement::SetSpriteCoords(const glm::vec4& sprite)
{
	spriteSize = sprite;
	renderableComponent->isGenerated = false;
}

void UIElement::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
	auto* guiProvider = dynamic_cast<GUIRederableProvider*>(renderableProvider);

	guiProvider->textureHandle = ResourceManager::Get().textures.GetHandle(textureName);
	guiProvider->materialHandle = ResourceManager::Get().materials.GetHandle("guiBase");
	guiProvider->uvRect = spriteSize;
	guiProvider->modelMatrix = uiTransformComponent->worldMatrix;
	guiProvider->zOrder = uiTransformComponent->zOrder;

	guiProvider->GenerateRenderables(outRenderables);
}

void UIElement::UpdateTransform(const glm::mat4& newTransform)
{
	for (auto& renderable : renderableComponent->renderables)
	{
		renderable.modelMatrix = newTransform;
	}
}

