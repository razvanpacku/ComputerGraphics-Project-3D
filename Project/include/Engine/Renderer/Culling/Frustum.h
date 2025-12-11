#pragma once
#include <glm/glm.hpp>
#include "BoundingBox.h"

// =========================================================
// Plane
//
// Represents a plane in 3D space using the plane equation.
// =========================================================
struct Plane
{
	glm::vec3 normal = glm::vec3(0.f);
	float d = 0; // distance from origin
};

// =========================================================
// Frustrum
//
// Represents a view frustum defined by six planes.
// =========================================================
struct Frustum
{
	Plane top;
	Plane bottom;

	Plane right;
	Plane left;

	Plane far;
	Plane near;

	Frustum() = default;
	Frustum(const glm::mat4& projectionViewMatrix);
};

bool AABBInFrustum(const Frustum& frustum, const BoundingBox& box);

