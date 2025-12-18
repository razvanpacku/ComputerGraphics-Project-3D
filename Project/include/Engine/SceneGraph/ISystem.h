#pragma once
#include <string>
#include "Scene.h"

// ======================================================
// ISystem
//
// Interface for systems that operate on the scene.
// ======================================================
class ISystem
{
public:
	ISystem(Scene* scene);
	virtual ~ISystem() = default;
	virtual void OnUpdate(double deltaTime) = 0;

	virtual std::string GetName() const = 0;
	bool ShouldRunOnStartup() const { return runOnStartup; }
protected:
	Scene* const scene;
	bool runOnStartup = false;
};
