#pragma once
#include "TransformEntity.h"

// ================================================================
// Anchor
// 
// Entity that represents an anchor point in the scene. Essentialy just a wrapper around TransformEntity.
// ================================================================

class Anchor : public TransformEntity
{
public:
	Anchor(const std::string& name = "Anchor")
		: TransformEntity(name)
	{
	}
};

