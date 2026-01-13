#pragma once
#include <glad/glad.h>
#include <optional>

#include "ResourceManagerTemplate.h"

struct Texture : public IResource {
	Texture() = default;
	~Texture() override = default;

	bool LoadFromFile(const std::string& path, bool generateMipmaps = true);
	void Destroy();

	void Bind(GLuint unit = 0, bool bindToUnit = true) const;

	// empty texture creator used by TextureManager helpers
	bool CreateEmpty2D(int w, int h, GLenum internalFormat, GLenum format, GLenum type,
		GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST,
		GLint wrapS = GL_CLAMP_TO_EDGE, GLint wrapT = GL_CLAMP_TO_EDGE, bool PCF = false);

	bool CreateEmptyCubemap(int size, GLenum internalFormat, GLenum format, GLenum type,
		GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST,
		GLint wrap = GL_CLAMP_TO_EDGE, bool PCF = false);

	bool CreateEmpty2DArray(int w, int h, int depth, GLenum internalFormat, GLenum format, GLenum type,
		GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST,
		GLint wrap = GL_CLAMP_TO_EDGE, bool PCF = false);

	GLuint id = 0;
	uint16_t width = 0;
	uint16_t height = 0;
	uint8_t channels = 0;

	GLenum target = GL_TEXTURE_2D;        // GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
	GLenum internalFormat = GL_RGBA8;     // e.g. GL_DEPTH_COMPONENT24 or GL_RGBA8

	uint16_t GetWidth() const { return width; }
	uint16_t GetHeight() const { return height; }
};

struct TextureResourceInfo {
	std::string path;
	bool generateMipmaps = true;
};

class TexturePolicy : public IResourcePolicy<Texture, TextureResourceInfo> {
public:
	using ResourceInfo = TextureResourceInfo;
	using ResourceType = Texture;

	Texture Create(const std::string& name, const TextureResourceInfo& resourceInfo) override;
	void Destroy(Texture& res) override;
};

class TextureManager : public ResourceManagerTemplate<Texture, TexturePolicy> {
public:
	using TextureHandle = Handle;

	TextureManager();
	~TextureManager();

	// Creates a 2D color texture
	TextureHandle CreateEmptyTexture2D(const std::string& name,
		int width, int height,
		GLenum internalFormat = GL_RGBA8,
		GLenum format = GL_RGBA,
		GLenum type = GL_UNSIGNED_BYTE,
		bool createSampler = true);

	// Creates a 2D depth texture suitable for shadow maps
	TextureHandle CreateDepthTexture2D(const std::string& name,
		int width, int height,
		GLenum depthInternalFormat = GL_DEPTH_COMPONENT24);

	// Creates a cube-map depth texture (for point light shadows)
	TextureHandle CreateDepthCubemap(const std::string& name, int size,
		GLenum depthInternalFormat = GL_DEPTH_COMPONENT24);

	TextureHandle CreateDepthTexture2DArray(const std::string& name,
		int width, int height, int depth,
		GLenum depthInternalFormat = GL_DEPTH_COMPONENT24);

	bool Bind(const TextureHandle& h, GLint unit, bool bindToUnit = true);
	bool Bind(const std::string& name, GLint unit, bool bindToUnit = true);

	void Unbind(const TextureHandle& h, bool bindToUnit = true);
	void Unbind(const std::string& name, bool bindToUnit = true);

	int GetBoundUnit(const TextureHandle& h) const;
	int GetBoundUnit(const std::string& name) const;

	void UnbindFromUnit(int unit, bool bindToUnit = true);
	void UnbindAll();

	virtual void PreloadResources(const std::string& resourceDirectory) override;
private:

	// OpenGL texture unit tracking:
	//   index = texture unit
	//   value = optional handle stored there
	std::vector<std::optional<TextureHandle>> unitToHandle;

	// Reverse lookup: handle -> texture unit
	std::unordered_map<TextureHandle, int> handleToUnit;

	int maxUnits = 0;
};