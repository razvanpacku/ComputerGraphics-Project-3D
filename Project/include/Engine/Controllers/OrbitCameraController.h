#pragma once
#include "Engine/Renderer/IRenderCamera.h"
#include "Engine/InputManager.h"

class OrbitCameraController : public IRenderCamera, public ConnectionHolder
{
public:
	OrbitCameraController(glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = 0.0f, float pitch = 0.0f, float sensitivity = 0.5f);
	~OrbitCameraController() override = default;
	glm::mat4 GetViewMatrix() const override;
	glm::mat4 GetProjectionMatrix() const override;
	void SetPosition(const glm::vec3& position) override;
	void SetOrientation(float yaw, float pitch) override;
	void Move(const glm::vec3& direction, float deltaTime) override {};

	glm::vec3 GetPosition() const override { return position; }

private:
	glm::vec3 target;

	glm::vec3 position;
	float yaw, pitch;
	float sensitivity;

	float radius = 3.0f;

	glm::vec3 front, right, up;

	static const glm::vec3 worldUp;

	bool rightMouseButtonDown = false;

	void UpdateCameraVectors();
};

