#pragma once
#include <glm/glm.hpp>

#include <array>

// =========================================================
// BoundingBox
//
// Axis-Aligned Bounding Box (AABB) representation and related functions.
// =========================================================
struct BoundingBox
{
	glm::vec3 min = glm::vec3(0.f);
	glm::vec3 max = glm::vec3(0.f);

	std::array<glm::vec3, 8> GetCorners() const;
};

bool IntersectAABB(const BoundingBox& a, const BoundingBox& b);
BoundingBox TransformAABB(const BoundingBox& box, const glm::mat4& transform);

