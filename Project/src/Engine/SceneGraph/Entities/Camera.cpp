#include "Engine/SceneGraph/Entities/Camera.h"

#include "Engine/Controllers/FlyingCameraController.h"

IRenderCamera* CreateRenderCamera(CameraType type)
{
	switch (type) {
	case CameraType::FLYING_CAMERA:
		return new FlyingCameraController();
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
