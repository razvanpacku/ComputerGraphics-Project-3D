#pragma once
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"

class ResourceManager
{
public:
    static ResourceManager& Get();

    ShaderManager shaders;
	TextureManager textures;
	UboManager ubos;
	MaterialManager materials;
	MeshManager meshes;
private:
    ResourceManager() = default;
};

