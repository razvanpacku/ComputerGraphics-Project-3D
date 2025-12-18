#pragma once
#include "../Entity.h"
#include "Engine/Renderer/IRenderCamera.h"
#include "Engine/Components/CameraComponent.h"

//forward declarations
class RenderSystem;

// ======================================================
// Camera
//
// Represents the camera through which the scene is viewed.
// It stores a RenderCamera, which is transmitted to the rendering engine.
// ======================================================
class Camera : public Entity
{
public:
	Camera(CameraType type = CameraType::FLYING_CAMERA, const std::string& name = "Camera");
	~Camera();

	void SwitchCameraType(CameraType newType);
	CameraType GetCameraType() const;

	// Sets the position of the camera.
	void SetPosition(const glm::vec3& position) { renderCamera->SetPosition(position); }
	// Sets the orientation of the camera using yaw and pitch angles.
	void SetOrientation(float yaw, float pitch) { renderCamera->SetOrientation(yaw, pitch); }
	// Moves the camera in the specified direction.
	void Move(const glm::vec3& direction, float deltaTime) { renderCamera->Move(direction, deltaTime); }
private:
	IRenderCamera* renderCamera;

	friend class RenderSystem;
};

