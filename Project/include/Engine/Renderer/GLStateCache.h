#pragma once
#include <glad/glad.h>

#include "Engine/Resources/ResourceManager.h"

// =========================================================
// GLStateCache
//
// Caches OpenGL state to minimize redundant state changes.
// =========================================================
struct GLStateCache {
	ShaderManager::Handle currentShader = ShaderManager::Handle{};
	Material* currentMaterial = nullptr;
	MeshManager::Handle currentMesh = MeshManager::Handle{};
	Mesh* currentDynamicMesh = nullptr;
	std::vector<TextureManager::Handle> boundTextures;

	bool cullBackfaces = true;

	GLint activeTextureUnit = -1;

	// In practice, there should be only one instance of this class owned by the Renderer, so one extra opengl call during its construction is acceptable.
	GLStateCache() {
		int maxTextureUnits = 0;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
		boundTextures.resize(maxTextureUnits, TextureManager::Handle{});
	}

	void Reset() {
		currentShader = ShaderManager::Handle{};
		currentMesh = MeshManager::Handle{};
		currentDynamicMesh = nullptr;
		boundTextures.clear();
		activeTextureUnit = -1;
	}
};