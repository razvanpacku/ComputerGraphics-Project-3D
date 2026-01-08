#include "Engine/SceneGraph/Entities/Textbox.h"

#include "Engine/Renderer/RenderableProvider/GUIRenderableProvider.h"

#include "Engine/SceneGraph/Systems/UITransformSystem.h"

glm::mat4 charMatrix(const CharData& data,const UITransformComponent& tansfC, float fontSize, const glm::vec2 screenSize) {
    return TransformFunctions::UIComputeLocal(
        {data.localPos.x, -data.localPos.y},
        glm::vec2(0.f, 1.0f),
        glm::vec2(16 * fontSize),
        glm::vec2(0.f),
        0.0f,
        glm::vec2(0.0f, 1.0f),
        tansfC.worldSize,
        screenSize
    );
}

// ================================================================
// Textbox
// ================================================================

Textbox::Textbox(const std::string& text, const std::string& name)
	: Entity(name), RenderEntity(name), UITransformEntity(name), text(text)
{
	renderableProvider = new GUIRederableProvider();
}

void Textbox::SetText(const std::string& newText)
{
	text = newText;
	renderableComponent->isGenerated = false;
}

void Textbox::SetFontSize(float newFontSize)
{
	fontSize = newFontSize;
	renderableComponent->isGenerated = false;
}

void Textbox::SetWrapWords(bool state)
{
	wrapWords = state;
	renderableComponent->isGenerated = false;
}

void Textbox::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
	auto* guiProvider = dynamic_cast<GUIRederableProvider*>(renderableProvider);
	guiProvider->textureHandle = ResourceManager::Get().textures.GetHandle("font.png");
	guiProvider->materialHandle = ResourceManager::Get().materials.GetHandle("guiBase");
	guiProvider->zOrder = uiTransformComponent->zOrder;
    guiProvider->GenerateRenderables(outRenderables);

    Renderable r = outRenderables[0];
    outRenderables.clear();
	chars.clear();

	float charPx = 16.0f * fontSize;
	float cursorX = padding.x;
	float cursorY = padding.y;
	glm::vec2 pixelScale = GetPixelScale();

    auto NewLine = [&](float& x, float& y) -> bool {
        x = padding.x;
        y += charPx;
        return y + charPx <= pixelScale.y - padding.y; // return false if exceeded vertical bound
        };

	for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];

        // handle newlines
        if (c == '\n') {
            if (!NewLine(cursorX, cursorY))
                break;
            continue;
        }
        if (c == '\r') { i++; continue; }

        // handle tabs
        if (c == '\t') {
            float tabWidth = 4 * charPx;
            cursorX += tabWidth - fmod(cursorX - padding.x, tabWidth);
            if (cursorX + charPx > pixelScale.x - padding.x) {
                if (!NewLine(cursorX, cursorY))
                    break;
            }
            continue;
        }

        // handle word wrapping
        if (wrapWords && !isspace((unsigned char)c)) {
            size_t wordStart = i;
            while (i < text.size() && !isspace((unsigned char)text[i]) && text[i] != '\n')
                i++;
            std::string word = text.substr(wordStart, i - wordStart);
            i--;
            float wordWidth = word.size() * charPx;
            float maxWidth = pixelScale.x - padding.x;

            bool atLineStart = (cursorX == padding.x);

            // Normal wrap to next line
            if (!atLineStart && cursorX + wordWidth > maxWidth) {
                if (!NewLine(cursorX, cursorY))
                    break; // no more room vertically

                atLineStart = true;
            }

            // Word still too long even at start split across lines
            if (atLineStart && wordWidth > (maxWidth - padding.x)) {
                for (char wc : word) {
                    if (cursorY + charPx > pixelScale.y - padding.y)
                        break; // vertical overflow
                    chars.push_back({ wc, glm::vec2(cursorX, cursorY) });
                    cursorX += charPx;

                    if (cursorX + charPx > pixelScale.x - padding.x) {
                        if (!NewLine(cursorX, cursorY))
                            break;
                    }
                }
                continue;
            }

            // Normal place-whole-word
            for (char wc : word) {
                if (cursorY + charPx > pixelScale.y - padding.y)
                    break;
                chars.push_back({ wc, glm::vec2(cursorX, cursorY) });
                cursorX += charPx;
            }
            continue;
        }

        //space character at the start of a new line is skipped
		if (wrapWords && isspace((unsigned char)c) && cursorX == padding.x) continue;

        // normal character
        if (cursorY >= pixelScale.y - padding.y) break; // stop if vertical overflow
        chars.push_back({ c, glm::vec2(cursorX, cursorY)});
        cursorX += charPx;

        if (cursorX + charPx > pixelScale.x - padding.x) {
            if (!NewLine(cursorX, cursorY)) break;
        }
	}

    UITransformComponent transC = GetComponent<UITransformComponent>();
    glm::mat4 entityMatrix = transC.worldMatrix;
    for (auto& ch : chars) {
        guiProvider->uvRect = {
            (ch.character % 16) / 16.f,
            1.f - ((ch.character / 16 + 1) / 16.f),
            1.f / 16.f,
            1.f / 16.f
        };
        guiProvider->modelMatrix = entityMatrix * charMatrix(ch, transC, fontSize, { GetSystem<UITransformSystem>()->ScreenWidth(), GetSystem<UITransformSystem>()->ScreenHeight() });

        guiProvider->GenerateRenderables(outRenderables);
    }


}

void Textbox::UpdateTransform(const glm::mat4& newTransform)
{
    renderableComponent->isGenerated = false;
}
