#pragma once
#include "../ISystem.h"
#include "Engine/Renderer/IRenderCamera.h"
#include "Engine/SceneGraph/Entities/Camera.h"
#include "Engine/SceneGraph/Entities/Light.h"

// forward declarations
class Renderer;

class RenderSystem : public ISystem
{
public:
	RenderSystem(Scene* scene, Renderer* renderer);
	~RenderSystem() override = default;
	void OnUpdate(double deltaTime) override;

	virtual std::string GetName() const override { return "RenderSystem"; }
private:
	Renderer* renderer = nullptr;

	CameraComponent* cameraComponent = nullptr;	
	LightComponent* lightComponent = nullptr;
};

