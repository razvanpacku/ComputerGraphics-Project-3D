#pragma once
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboManager.h"

class ResourceManager
{
public:
    static ResourceManager& Get();

    ShaderManager shaders;
	TextureManager textures;
	UboManager ubos;
private:
    ResourceManager() = default;
};

