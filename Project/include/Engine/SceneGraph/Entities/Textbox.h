#pragma once
#include "UITransformEntity.h"
#include "RenderEntity.h"

struct CharData {
	char character;
	glm::vec2 localPos;
};

// ================================================================
// Textbox
//
// Represents a textbox UI element in the scene.
// ================================================================
class Textbox : public RenderEntity, public UITransformEntity
{
public:
	Textbox(const std::string& text, const std::string& name = "Textbox");

	std::string GetText() const { return text; }
	float GetFontSize() const { return fontSize; }
	bool GetWrapWords() const { return wrapWords; }

	void SetText(const std::string& newText);
	void SetFontSize(float newFontSize);
	void SetWrapWords(bool state);
	
private:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;

	std::vector<CharData> chars;

	std::string text;
	float fontSize = 1.0f; // relative to the font's base 16x16 size for each character
	bool wrapWords = true; // whether to wrap words or cut off

	glm::vec2 padding = glm::vec2(0.f);
};

