#pragma once
#include "Engine/Resources/ResourceManager.h"

//forward declaration
struct GLStateCache;

enum class ShadowMapType
{
	Directional,
	Point
};

// =========================================================
// ShadowFramebuffer
//
// Owns a framebuffer object + depth texture for shadow mapping.
// Two of these are used in the Renderer, one for directional and one for point lights.
// =========================================================
class ShadowFramebuffer
{
public:
	ShadowFramebuffer(ShadowMapType type);
	~ShadowFramebuffer();

	// Bind this FBO for rendering (shadow pass). For cube map faces, faceIndex in [0..5]
	void BindForWriting(int faceIndex = -1) const;

	// Unbind (bind default framebuffer)
	// Caller is responsible for restoring viewport if needed
	static void Unbind(const GLStateCache& glState);

	void Clear() const;

	TextureManager::TextureHandle GetTextureHandle() const { return texHandle; }

	bool IsCube() const { return isCube; }
	int GetSize() const { return size; }
private:
	GLuint fbo = 0;
	TextureManager::TextureHandle texHandle;
	bool isCube = false;
	int size = 0;

};

