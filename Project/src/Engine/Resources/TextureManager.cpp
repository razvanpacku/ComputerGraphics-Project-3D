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
	glBindTexture(target, id);
}

// Create empty 2D texture
bool Texture::CreateEmpty2D(int w, int h, GLenum inFormat, GLenum format, GLenum type,
    GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT, bool PCF)
{
    width = static_cast<uint16_t>(w);
    height = static_cast<uint16_t>(h);
    channels = 0;
    target = GL_TEXTURE_2D;
    internalFormat = inFormat;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    // allocate
    glTexImage2D(GL_TEXTURE_2D, 0, inFormat, width, height, 0, format, type, nullptr);

    // params

    if (PCF) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

// Create empty cubemap texture
bool Texture::CreateEmptyCubemap(int size, GLenum inFormat, GLenum format, GLenum type,
    GLint minFilter, GLint magFilter, GLint wrap, bool PCF)
{
    width = static_cast<uint16_t>(size);
    height = static_cast<uint16_t>(size);
    channels = 0;
    target = GL_TEXTURE_CUBE_MAP;
    internalFormat = inFormat;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, inFormat,
            width, height, 0, format, type, nullptr);
    }
    if (PCF) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return true;
}

//Create empty 2D texture array
bool Texture::CreateEmpty2DArray(int w, int h, int depth, GLenum inFormat, GLenum format, GLenum type,
    GLint minFilter, GLint magFilter, GLint wrap, bool PCF) {
	width = static_cast<uint16_t>(w);
	height = static_cast<uint16_t>(h);
    channels = 0;
	target = GL_TEXTURE_2D_ARRAY;
	internalFormat = inFormat;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id);

	// allocate
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, inFormat, width, height, depth, 0, format, type, nullptr);

	// params

    if (PCF) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, wrap);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return true;
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

// --- Creation helpers ---

TextureManager::TextureHandle TextureManager::CreateEmptyTexture2D(const std::string& name,
    int width, int height,
    GLenum internalFormat,
    GLenum format,
    GLenum type,
    bool createSampler)
{
    Texture tex;
    if (!tex.CreateEmpty2D(width, height, internalFormat, format, type,
        GL_NEAREST, GL_NEAREST,
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE))
    {
        std::cerr << "Failed to create empty texture: " << name << "\n";
        return {};
    }

    tex.alive = true;
    return Register(name, tex);
}

TextureManager::TextureHandle TextureManager::CreateDepthTexture2D(const std::string& name,
    int width, int height,
    GLenum depthInternalFormat)
{
    GLenum format = GL_DEPTH_COMPONENT;             // GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, or GL_DEPTH_COMPONENT32F
    GLenum type = GL_FLOAT;
    Texture tex;

	std::cout << "Creating depth texture: " << name << " (" << width << "x" << height << ")\n";

    if (!tex.CreateEmpty2D(width, height, depthInternalFormat, format, GL_FLOAT,
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, true))
    {
        std::cerr << "Failed to create depth texture: " << name << "\n";
        return {};
    }

    // For depth maps we want border color 1.0 and clamp-to-border
    glBindTexture(GL_TEXTURE_2D, tex.id);
    float border[4] = { 1.f, 1.f, 1.f, 1.f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindTexture(GL_TEXTURE_2D, 0);

    tex.alive = true;
    return Register(name, tex);
}

TextureManager::TextureHandle TextureManager::CreateDepthCubemap(const std::string& name, int size,
    GLenum depthInternalFormat)
{
    Texture tex;

	std::cout << "Creating depth cubemap: " << name << " (" << size << "x" << size << ")\n";

    if (!tex.CreateEmptyCubemap(size, depthInternalFormat, GL_DEPTH_COMPONENT, GL_FLOAT,
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, true))
    {
        std::cerr << "Failed to create depth cubemap: " << name << "\n";
        return {};
    }

    // For depth maps we want border color 1.0 and clamp-to-border
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex.id);
    float border[4] = { 1.f,1.f,1.f,1.f };
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    tex.alive = true;
    return Register(name, tex);
}

TextureManager::TextureHandle TextureManager::CreateDepthTexture2DArray(const std::string& name,
    int width, int height, int depth,
    GLenum depthInternalFormat)
{
    GLenum format = GL_DEPTH_COMPONENT;             // GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, or GL_DEPTH_COMPONENT32F
    GLenum type = GL_FLOAT;
    Texture tex;
    std::cout << "Creating depth texture array: " << name << " (" << width << "x" << height << "x" << depth << ")\n";
    if (!tex.CreateEmpty2DArray(width, height, depth, depthInternalFormat, format, type,
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, true))
    {
        std::cerr << "Failed to create depth texture array: " << name << "\n";
        return {};
    }
    // For depth maps we want border color 1.0 and clamp-to-border
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex.id);
    float border[4] = { 1.f, 1.f, 1.f, 1.f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    tex.alive = true;
    return Register(name, tex);
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
            if (relativePath == "font.png") {
				tri.generateMipmaps = false;
				Load(relativePath, tri);
				tri.generateMipmaps = true;
                continue;
            }
			Load(relativePath, tri);
        }
	}

	// create the textures for shadow maps
# define SHADOW_MAP_SIZE 2048
# define SHADOW_CASCADE_COUNT 6
	CreateDepthCubemap("shadow/point", SHADOW_MAP_SIZE, GL_DEPTH_COMPONENT32F);
	std::string shadowDirPrefix = "shadow/dir";
	CreateDepthTexture2DArray(shadowDirPrefix, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, SHADOW_CASCADE_COUNT, GL_DEPTH_COMPONENT32F);
}