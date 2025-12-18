#include "Engine/Controllers/FlyingCameraController.h"

#include "Engine/App.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

const glm::vec3 FlyingCameraController::worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

FlyingCameraController::FlyingCameraController(glm::vec3 postion, float yaw, float pitch, float cameraSpeed, float sensitivity)
	: position(postion), yaw(glm::clamp(yaw, 0.f, 360.f)), pitch(glm::clamp(pitch, -89.f, 89.f)), cameraSpeed(cameraSpeed), sensitivity(sensitivity)
{
	UpdateCameraVectors();

	// bind inputs
	auto& _im = InputManager::Get();

	connections.reserve(8);

	connections.emplace_back(_im.BindMouseDelta([this, &_im](double dx, double dy) {
		if (_im.GetMouseMode() != MouseMode::Disabled)
			return;

		if (dx != 0 || dy != 0) {
			this->yaw += (float)dx * this->sensitivity;
			this->pitch -= (float)dy * this->sensitivity;
			if (this->pitch > 89.0f)
				this->pitch = 89.0f;
			else if (this->pitch < -89.0f)
				this->pitch = -89.0f;
			if(this->yaw > 360.0f)
				this->yaw -= 360.0f;
			else if(this->yaw < 0.0f)
				this->yaw += 360.0f;
		}
		UpdateCameraVectors();
		}));

	connections.emplace_back(_im.BindMouseScroll([this](double xoffset, double yoffset) {
		double scale = glm::pow(1.1, yoffset);
		this->cameraSpeed *= (float)scale;
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_W, InputEventType::Held, [this]() {
		ProcessCameraMovement(FORWARD, ((float)App::Get().DeltaTime()));
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_S, InputEventType::Held, [this]() {
		ProcessCameraMovement(BACKWARD, ((float)App::Get().DeltaTime()));
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_A, InputEventType::Held, [this]() {
		ProcessCameraMovement(LEFT, ((float)App::Get().DeltaTime()));
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_D, InputEventType::Held, [this]() {
		ProcessCameraMovement(RIGHT, ((float)App::Get().DeltaTime()));
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_SPACE, InputEventType::Held, [this]() {
		ProcessCameraMovement(UP, ((float)App::Get().DeltaTime()));
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_LEFT_SHIFT, InputEventType::Held, [this]() {
		ProcessCameraMovement(DOWN, ((float)App::Get().DeltaTime()));
		}));

	InputManager::Get().BindKey(GLFW_KEY_C, InputEventType::Pressed, []() {
		//toggle mouse mode between disabled and normal
		auto& _im = InputManager::Get();
		if (_im.GetMouseMode() == MouseMode::Disabled)
			_im.SetMouseMode(MouseMode::Normal);
		else
			_im.SetMouseMode(MouseMode::Disabled);
		});


	// set mouse mode to disabled initially
	_im.SetMouseMode(MouseMode::Disabled);
}

glm::mat4 FlyingCameraController::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 FlyingCameraController::GetProjectionMatrix() const
{
	return glm::infinitePerspective(glm::radians(90.0f), App::Get().GetWindowAspectRatio(), nearPlane);
}

void FlyingCameraController::SetPosition(const glm::vec3& pos)
{
	position = pos;
}

void FlyingCameraController::SetOrientation(float newYaw, float newPitch)
{
	yaw = newYaw;
	pitch = newPitch;
	UpdateCameraVectors();
}

void FlyingCameraController::Move(const glm::vec3& direction, float deltaTime)
{
	glm::vec3 moveDir = glm::normalize(direction);
	position += moveDir * cameraSpeed * deltaTime;
}

void FlyingCameraController::UpdateCameraVectors()
{
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void FlyingCameraController::ProcessCameraMovement(Camera_Movement dir, float deltaTime)
{
	float velocity = cameraSpeed * deltaTime;
	glm::vec3 direction(0.0f);
	if (dir == FORWARD)  direction += front;
	if (dir == BACKWARD) direction -= front;
	if (dir == LEFT)     direction -= right;
	if (dir == RIGHT)    direction += right;
	if (dir == UP)       direction += up;
	if (dir == DOWN)     direction -= up;
	direction = glm::normalize(direction);
	position += direction * velocity;
}
