#pragma once
#include "Engine/Resources/ShaderManager.h"

class ResourceManager
{
public:
    static ResourceManager& Get();

    ShaderManager shaders;
private:
    ResourceManager() = default;
};

