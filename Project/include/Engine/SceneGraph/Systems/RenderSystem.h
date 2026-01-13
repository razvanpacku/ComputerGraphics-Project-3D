#pragma once
#include "../ISystem.h"
#include "Engine/Renderer/IRenderCamera.h"
#include "Engine/SceneGraph/Entities/Camera.h"
#include "Engine/SceneGraph/Entities/Light.h"

#include "Engine/Components/RenderableComponent.h"

// forward declarations
class Renderer;

class RenderSystem : public ISystem
{
public:
	RenderSystem(Scene* scene, int16_t order, Renderer* renderer, entt::registry* registry = nullptr);
	~RenderSystem() override = default;
	void OnUpdate(double deltaTime) override;

	virtual std::string GetName() const override { return "RenderSystem"; }

	void UpdateTargetCamera(glm::vec3 targetPosition); // called by the TransformSystem when the target entity's transform is updated
private:
	Renderer* renderer = nullptr;
	entt::registry* registry = nullptr;

	CameraComponent* cameraComponent = nullptr;	
	LightComponent* lightComponent = nullptr;
};

