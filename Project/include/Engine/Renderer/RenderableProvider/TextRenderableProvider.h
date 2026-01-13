#pragma once
#include "IRendarableProvider.h"
#include "../../Resources/ResourceManager.h"
#include "Engine/DataStructures/Transform.h"

#include "GUIRenderableProvider.h"

#define TEXTURE_FONT_SIZE 16 // font texture is 16x16 characters
#define TEXTURE_FONT_COLS 16
#define TEXTURE_FONT_ROWS 16

class TextRenderableProvider : public IRendarableProvider
{
public:
	std::string_view text;
	float fontSize = 1.0f; // relative to the font's base 16x16 size for each character
	bool wrapWords = true; // whether to wrap words or cut off

	// these are for the bounding box of the text area
	glm::vec3 position = glm::vec3(0.f); // pixel offset
	glm::vec2 relativePosition = glm::vec2(0.5f, 0.5f); 
	glm::quat rotation = glm::quat(glm::vec3(0.f));
	// scale is given in pixels for wrapping purposes, because font size is in pixels
	// if scale was relative, we couldn't determine how many characters fit in the box sicne screen size can vary
	// future textbox entity will recreate renderables each frame
	glm::vec2 pixelScale = glm::vec2(256.f, 256.f);
	glm::vec2 anchorPoint = glm::vec2(0.5f, 0.5f); 

	glm::vec2 padding = glm::vec2(0.f); // in pixels
	int16_t zOrder = 0;

	void GenerateRenderables(std::vector<Renderable>& out) override
	{
		auto& _rm = ResourceManager::Get();
		if (!_rm.AreResourcesPreloaded()) return;
		if (fontProvider == nullptr)
		{
			fontProvider = new GUIRederableProvider();
			fontProvider->materialHandle = _rm.materials.GetHandle("guiBase");
			fontProvider->textureHandle = _rm.textures.GetHandle("font.png");
			//fontProvider->anchorPoint = glm::vec2(0.f, 1.f);
		}

		float charPx = TEXTURE_FONT_SIZE * fontSize;

		float cursorX = padding.x;
		float cursorY = padding.y;

		fontProvider->zOrder = zOrder;


		for (size_t i = 0; i < text.size();) {
			char c = text[i];
			if (c == '\n')
			{
				if (!NewLine(cursorX, cursorY, charPx))
					return;
				i++;
				continue;
			}

			if (c == '\r')
			{
				i++;
				continue;
			}

			if (c == '\t')
			{
				float tabWidth = 4 * charPx;
				cursorX += tabWidth - fmod(cursorX - padding.x, tabWidth);
				if (cursorX >= pixelScale.x - padding.x)
				{
					if (!NewLine(cursorX, cursorY, charPx))
						return;
				}
				i++;
				continue;
			}

			if (wrapWords && !isspace((unsigned char)c))
			{
				size_t wordStart = i;
				while (i < text.size() && !isspace((unsigned char)text[i]) && text[i] != '\n')
					i++;

				std::string_view word = text.substr(wordStart, i - wordStart);
				float wordWidth = MeasureWordWidth(word, charPx);

				if (cursorX + wordWidth > pixelScale.x - padding.x)
				{
					if (!NewLine(cursorX, cursorY, charPx))
						return;
				}

				// render the word character-by-character
				for (char wc : word)
				{
					RenderCharacter(wc, cursorX, cursorY, charPx, out);
					cursorX += charPx;
				}

				continue;
			}

			RenderCharacter(c, cursorX, cursorY, charPx, out);
			cursorX += charPx;

			if (cursorX >= pixelScale.x - padding.x)
			{
				if (!NewLine(cursorX, cursorY, charPx))
					return;
			}
			i++;
		}
	}
private:
	static GUIRederableProvider* fontProvider; // shared font provider for all text renderables

	float MeasureWordWidth(std::string_view word, float charPx) const
	{
		return word.size() * charPx;
	}

	bool NewLine(float& cursorX, float& cursorY, float charPx) const
	{
		cursorX = padding.x;
		cursorY += charPx;
		return cursorY < pixelScale.y - padding.y;
	}

	void RenderCharacter(
		char c,
		float cursorX,
		float cursorY,
		float charPx,
		std::vector<Renderable>& out)
	{
		glm::vec3 charPos = position +
			glm::vec3(cursorX, -cursorY, 0.f) -
			glm::vec3(pixelScale.x * anchorPoint.x,
				-pixelScale.y * (1.f - anchorPoint.y),
				0.f);

		glm::vec3 rotatedPos = position + rotation * (charPos - position);

		fontProvider->transform.position = rotatedPos;
		fontProvider->transform.rotation = rotation;
		fontProvider->transform.scale = glm::vec3(charPx, charPx, 1.f);
		//fontProvider->relativePosition = relativePosition;
		//fontProvider->relativeSize = { 0.f, 0.f };

		uint8_t asciiIndex = static_cast<uint8_t>(c);
		uint8_t texCol = asciiIndex % TEXTURE_FONT_COLS;
		uint8_t texRow = TEXTURE_FONT_ROWS - asciiIndex / TEXTURE_FONT_ROWS - 1;

		fontProvider->uvRect = {
			texCol / (float)TEXTURE_FONT_COLS,
			texRow / (float)TEXTURE_FONT_ROWS,
			1.f / TEXTURE_FONT_COLS,
			1.f / TEXTURE_FONT_ROWS
		};

		fontProvider->GenerateRenderables(out);
	}
};

GUIRederableProvider* TextRenderableProvider::fontProvider = nullptr;