#pragma once
#include "TransformEntity.h"
#include "UITransformEntity.h"

// ================================================================
// Anchor
// 
// Entity that represents an anchor point in the scene. Essentialy just a wrapper around TransformEntity.
// ================================================================

class Anchor : public TransformEntity
{
public:
	Anchor(const std::string& name = "Anchor")
		: Entity(name), TransformEntity(name)
	{
	}
};

// ================================================================
// UIAnchor
//
// Entity that represents an anchor point for UI elements. Essentially just a wrapper around UITransformEntity.
// ================================================================

class UIAnchor : public UITransformEntity
{
public:
	UIAnchor(const std::string& name = "UIAnchor")
		: Entity(name), UITransformEntity(name)
	{
	}
};

