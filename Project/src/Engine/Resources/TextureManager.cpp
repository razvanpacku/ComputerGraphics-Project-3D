#include "Engine/Resources/TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// =========================================================
// Texture
// =========================================================
bool Texture::LoadFromFile(const std::string& path, bool generateMipmaps)
{
    stbi_set_flip_vertically_on_load(1);

    int w = 0, h = 0, c = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 0);
    if (!data)
        return false;

    width = static_cast<uint16_t>(w);
    height = static_cast<uint16_t>(h);
    channels = static_cast<uint8_t>(c);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &id);


    // Save current active texture and binding so we can restore them
    GLint prevActive = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActive);

    // Activate texture unit 0 for safe uploads and save its current binding
    glActiveTexture(GL_TEXTURE0);
    GLint prevBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);

    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);

    if (generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        generateMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Restore previous binding and active unit to avoid side effects
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevBinding));
    glActiveTexture(static_cast<GLenum>(prevActive));

    stbi_image_free(data);
    return true;
}

void Texture::Destroy()
{
    if (id != 0) {
        glDeleteTextures(1, &id);
        id = 0;
    }
}

void Texture::Bind(GLuint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

// =========================================================
// TexturePolicy
// =========================================================
Texture TexturePolicy::Create(const std::string& name, const TextureResourceInfo& textureInfo)
{
    Texture texture;
    if (!texture.LoadFromFile(textureInfo.path, textureInfo.generateMipmaps))
    {
        std::cerr << "Failed to load texture: " << textureInfo.path << " for resource " << name << std::endl;
    }
    else
    {
        texture.alive = true;
    }
    return texture;
}

void TexturePolicy::Destroy(Texture& texture)
{
    texture.Destroy();
}

// =========================================================
// TextureManager
// =========================================================
TextureManager::TextureManager()
{
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
    unitToHandle.resize(maxUnits);
}

TextureManager::~TextureManager()
{
    UnbindAll();
}

// --- Utility ---
int TextureManager::FindFreeUnit() const
{
    for (int i = 0; i < maxUnits; i++)
        if (!unitToHandle[i].has_value())
            return i;

    return -1; // No free units
}

int TextureManager::GetBoundUnit(const TextureHandle& handle) const
{
    auto it = handleToUnit.find(handle);
    return (it == handleToUnit.end()) ? -1 : it->second;
}

int TextureManager::GetBoundUnit(const std::string& name) const
{
    auto h = GetHandle(name);
    return h.IsValid() ? GetBoundUnit(h) : -1;
}

// --- Binding ---
bool TextureManager::Bind(const TextureHandle& handle)
{
    if (!handle.IsValid())
        return false;

    // If already bound, do nothing
    int existingUnit = GetBoundUnit(handle);
    if (existingUnit != -1)
        return true;

    int unit = FindFreeUnit();
    if (unit == -1)
        return false; // No units available

    Texture* tex = Get(handle);
    if (!tex)
        return false;

    BindToUnit(tex, unit);

    unitToHandle[unit] = handle;
    handleToUnit[handle] = unit;

    return true;
}

bool TextureManager::Bind(const std::string& name)
{
    auto handle = GetHandle(name);
    return Bind(handle);
}

// --- Unbinding ---
void TextureManager::Unbind(const TextureHandle& handle)
{
    int unit = GetBoundUnit(handle);
    if (unit == -1)
        return;

    UnbindFromUnit(unit);

    unitToHandle[unit].reset();
    handleToUnit.erase(handle);
}

void TextureManager::Unbind(const std::string& name)
{
    auto h = GetHandle(name);
    if (h.IsValid())
        Unbind(h);
}

void TextureManager::UnbindAll()
{
    for (int i = 0; i < maxUnits; i++)
        if (unitToHandle[i].has_value())
            UnbindFromUnit(i);

    unitToHandle.clear();
    unitToHandle.resize(maxUnits);
    handleToUnit.clear();
}

// --- Internal binding helpers ---
void TextureManager::BindToUnit(Texture* texture, int unit)
{
    texture->Bind(unit);
}

void TextureManager::UnbindFromUnit(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}