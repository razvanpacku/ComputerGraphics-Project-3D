#pragma once
#include "Engine/SceneGraph/Scene.h"

// ======================================================
// TestScene
//
// A simple test scene for demonstration/development purposes.
// ======================================================
class TestScene : public Scene
{
public:
	void OnCreate() override;
	void OnUpdate(double deltaTime) override;
};

