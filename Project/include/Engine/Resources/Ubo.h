#pragma once
#include "ResourceManagerTemplate.h"
#include "ShaderReflection.h"

// This is put into a seperate file to avoid circular dependencies

struct Ubo : public IResource, public UniformBlockInfo {};