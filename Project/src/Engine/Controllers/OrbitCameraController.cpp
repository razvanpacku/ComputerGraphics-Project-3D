#include "Engine/Controllers/OrbitCameraController.h"

#include "Engine/App.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

const glm::vec3 OrbitCameraController::worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

OrbitCameraController::OrbitCameraController(glm::vec3 target, float yaw, float pitch, float sensitivity)
	: target(target), yaw(glm::clamp(yaw, 0.f, 360.f)), pitch(glm::clamp(pitch, -89.f, 89.f)), sensitivity(sensitivity)
{
	UpdateCameraVectors();

	//bind inputs
	auto& _im = InputManager::Get();

	connections.reserve(4);

	connections.emplace_back(_im.BindMouseDelta([this, &_im](double dx, double dy) {
		if (!rightMouseButtonDown) return;

		if (dx != 0 || dy != 0) {
			this->yaw += (float)dx * this->sensitivity;
			this->pitch += (float)dy * this->sensitivity;

			this->pitch = glm::clamp(this->pitch, -89.0f, 89.0f);

			if (this->yaw > 360.0f) this->yaw -= 360.0f;
			else if (this->yaw < 0.0f) this->yaw += 360.0f;
		}
		UpdateCameraVectors();
		}));

	connections.emplace_back(_im.BindMouseScroll([this](double, double yoffset) {
		float factor = glm::pow(1.1f, (float)yoffset);
		radius *= 1.0f / factor;
		radius = glm::clamp(radius, 0.5f, 100.0f);
		UpdateCameraVectors();
		}));

	connections.emplace_back(_im.BindMouseButton(GLFW_MOUSE_BUTTON_RIGHT, InputEventType::Pressed, [this]() {
		rightMouseButtonDown = true;
		}));

	connections.emplace_back(_im.BindMouseButton(GLFW_MOUSE_BUTTON_RIGHT, InputEventType::Released, [this]() {
		rightMouseButtonDown = false;
		}));

	_im.SetMouseMode(MouseMode::Normal);
}

glm::mat4 OrbitCameraController::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 OrbitCameraController::GetProjectionMatrix() const
{
	return glm::infinitePerspective(glm::radians(90.0f), App::Get().GetWindowAspectRatio(), nearPlane);
}

void OrbitCameraController::SetPosition(const glm::vec3& newTarget)
{
	target = newTarget;
	UpdateCameraVectors();
}

void OrbitCameraController::SetOrientation(float newYaw, float newPitch)
{
	yaw = newYaw;
	pitch = newPitch;
	UpdateCameraVectors();
}


void OrbitCameraController::UpdateCameraVectors()
{
	// Convert spherical coords (yaw,pitch,radius) to Cartesian
	glm::vec3 dir;
	dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	dir.y = sin(glm::radians(pitch));
	dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(-dir);

	position = target + dir * radius;

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

