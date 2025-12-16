#pragma once
#include <glm/glm.hpp>

// =========================================================
// IRenderCamera
//
// Interface for render camera implementations.
// =========================================================

class IRenderCamera
{
public:
	virtual ~IRenderCamera() = default;
	// Returns the view matrix of the camera.
	virtual glm::mat4 GetViewMatrix() const = 0;
	// Returns the projection matrix of the camera.
	virtual glm::mat4 GetProjectionMatrix() const = 0;
	// Sets the position of the camera.
	virtual void SetPosition(const glm::vec3& position) = 0;
	// Sets the orientation of the camera using yaw and pitch angles.
	virtual void SetOrientation(float yaw, float pitch) = 0;
	// Moves the camera in the specified direction.
	virtual void Move(const glm::vec3& direction, float deltaTime) = 0;

	// Used by cameras for which a position makes sense
	// Used by the renderer/shaders to get the camera position for lighting calculations
	virtual glm::vec3 GetPosition() const = 0;

protected:
	float nearPlane = 0.1f;
};

