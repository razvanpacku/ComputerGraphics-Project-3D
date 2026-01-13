#pragma once
#include "UITransformEntity.h"
#include "RenderEntity.h"

// ================================================================
// UIElement
//
// Represents a generic UI element in the scene.
// ================================================================
class UIElement : public RenderEntity, public UITransformEntity
{
public:
	UIElement(const std::string& texture = "",const glm::vec4& sprite = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), const std::string& name = "UIElement");

	const std::string& GetTexture() const { return textureName; }
	const glm::vec2& GetSpriteCoords() const { return spriteSize; }

	void SetTexture(const std::string& texture);
	void SetSpriteCoords(const glm::vec4& sprite);
private:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;
	std::string textureName;
	glm::vec4 spriteSize;
};

