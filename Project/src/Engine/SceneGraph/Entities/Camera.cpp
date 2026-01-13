#include "Engine/SceneGraph/Entities/Camera.h"

#include "Engine/Controllers/FlyingCameraController.h"
#include "Engine/Controllers/OrbitCameraController.h"

#include "Engine/SceneGraph/Systems/Includes.h"

IRenderCamera* CreateRenderCamera(CameraType type)
{
	switch (type) {
	case CameraType::FLYING_CAMERA:
		return new FlyingCameraController();
	case CameraType::ORBIT_CAMERA:
		return new OrbitCameraController();
	default:
		throw std::runtime_error("Unknown CameraType");
	}
}

// ======================================================
// Camera
// ======================================================

Camera::Camera(CameraType type, const std::string& name)
	: Entity(name)
{
	renderCamera = CreateRenderCamera(type);
	AddComponent<CameraComponent>(type, false);
}

Camera::~Camera()
{
	delete renderCamera;
}

void Camera::SwitchCameraType(CameraType newType)
{

	auto& camComp = GetComponent<CameraComponent>();
	if (newType == camComp.type) return;
	delete renderCamera;
	renderCamera = CreateRenderCamera(newType);
	camComp.type = newType;
	camComp.dirty = true;
}

CameraType Camera::GetCameraType() const
{
	auto& camComp = GetComponent<CameraComponent>();
	return camComp.type;
}

void Camera::SetTarget(Entity* targetEntity)
{
	auto* transformSystem = GetSystem<TransformSystem>();
	if (!transformSystem) {
		throw std::runtime_error("TransformSystem not found in scene");
	}

	if (targetEntity) {
		transformSystem->SetTarget(targetEntity->GetHandle());
	}
	else {
		transformSystem->SetTarget(entt::null);
	}
}