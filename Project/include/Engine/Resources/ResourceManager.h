#pragma once
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "ModelManager.h"

class ResourceManager
{
public:
    static ResourceManager& Get();

	void PreloadResources(const std::string& resourceDirectory);

    ShaderManager shaders;
	TextureManager textures;
	UboManager ubos;
	MaterialManager materials;
	MeshManager meshes;
	ModelManager models;
private:
    ResourceManager() = default;
};

