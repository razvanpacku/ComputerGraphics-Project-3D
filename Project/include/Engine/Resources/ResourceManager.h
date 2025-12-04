#pragma once
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboManager.h"
#include "MaterialManager.h"

class ResourceManager
{
public:
    static ResourceManager& Get();

    ShaderManager shaders;
	TextureManager textures;
	UboManager ubos;
	MaterialManager materials;
private:
    ResourceManager() = default;
};

