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

void Texture::Bind(GLuint unit, bool bindToUnit) const
{
    if(bindToUnit) glActiveTexture(GL_TEXTURE0 + unit);
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
	texture.alive = false;
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
bool TextureManager::Bind(const TextureHandle& handle, GLint unit, bool bindToUnit)
{
    if (!handle.IsValid())
        return false;

	//if already bound to that unit, do nothing
	if (unit == GetBoundUnit(handle))
		return true;

    Texture* tex = Get(handle);
    if (!tex)
        return false;

	// If the unit is already occupied, unbind the existing texture
    if (unitToHandle[unit].has_value())
    {
        TextureHandle existingHandle = unitToHandle[unit].value();
        handleToUnit.erase(existingHandle);
	}

	tex->Bind(unit, bindToUnit);

    unitToHandle[unit] = handle;
    handleToUnit[handle] = unit;

    return true;
}

bool TextureManager::Bind(const std::string& name, GLint unit, bool bindToUnit)
{
    auto handle = GetHandle(name);
    return Bind(handle, unit, bindToUnit);
}

// --- Unbinding ---
void TextureManager::Unbind(const TextureHandle& handle, bool bindToUnit)
{
    int unit = GetBoundUnit(handle);
    if (unit == -1)
        return;

    UnbindFromUnit(unit, bindToUnit);

    unitToHandle[unit].reset();
    handleToUnit.erase(handle);
}

void TextureManager::Unbind(const std::string& name, bool bindToUnit)
{
    auto h = GetHandle(name);
    if (h.IsValid())
        Unbind(h, bindToUnit);
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

void TextureManager::UnbindFromUnit(int unit, bool bindToUnit)
{
    if(bindToUnit) glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (unitToHandle[unit].has_value())
    {
        TextureHandle handle = unitToHandle[unit].value();
        handleToUnit.erase(handle);
        unitToHandle[unit].reset();
	}
}

// --- Preloading ---
void TextureManager::PreloadResources(const std::string& resourceDirectory)
{
	std::string texturesDir = "textures/";
    std::filesystem::path fullDir = std::filesystem::path(resourceDirectory) / texturesDir;

	// Iterate recursively over files in the textures directory
	// name will be relative path from texturesDir
    for (const auto& entry : std::filesystem::recursive_directory_iterator(fullDir)) {
        if (entry.is_regular_file()) {
            std::string fullPath = entry.path().string();
            std::string relativePath = std::filesystem::relative(entry.path(), fullDir).string();
            TextureResourceInfo tri;
            std::cout << "Loading texture: " << relativePath << "\n";
            tri.path = fullPath;
            tri.generateMipmaps = true;
			Load(relativePath, tri);
        }
	}
}