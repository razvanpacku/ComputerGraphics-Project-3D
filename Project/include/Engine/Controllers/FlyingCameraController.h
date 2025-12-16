#pragma once
#include "Engine/Renderer/IRenderCamera.h"
#include "Engine/InputManager.h"

class FlyingCameraController : public IRenderCamera, public ConnectionHolder
{
public:
	FlyingCameraController(glm::vec3 postion = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = 0.0f, float pitch = 0.0f, float cameraSpeed = 2.5f, float sensitivity = 0.1f);
	~FlyingCameraController() override = default;
	glm::mat4 GetViewMatrix() const override;
	glm::mat4 GetProjectionMatrix() const override;
	void SetPosition(const glm::vec3& position) override;
	void SetOrientation(float yaw, float pitch) override;
	void Move(const glm::vec3& direction, float deltaTime) override;

	glm::vec3 GetPosition() const override { return position; }
private:
	glm::vec3 position;
	float yaw, pitch;
	float cameraSpeed, sensitivity;

	glm::vec3 front, right, up;

	static const glm::vec3 worldUp;

	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	void ProcessCameraMovement(Camera_Movement dir, float deltaTime);
	void UpdateCameraVectors();
};

