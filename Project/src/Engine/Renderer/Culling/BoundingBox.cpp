#include "Engine/Renderer/Culling/BoundingBox.h"

std::array<glm::vec3, 8> BoundingBox::GetCorners() const
{
	return {
		glm::vec3(min.x, min.y, min.z),
		glm::vec3(max.x, min.y, min.z),
		glm::vec3(min.x, max.y, min.z),
		glm::vec3(max.x, max.y, min.z),
		glm::vec3(min.x, min.y, max.z),
		glm::vec3(max.x, min.y, max.z),
		glm::vec3(min.x, max.y, max.z),
		glm::vec3(max.x, max.y, max.z)
	};
}

bool IntersectAABB(const BoundingBox& a, const BoundingBox& b)
{
	// Check for overlap on each axis
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
	return true; // Overlap on all axes
}

BoundingBox TransformAABB(const BoundingBox& box, const glm::mat4& transform)
{
	// Get the 8 corners of the original AABB
	auto corners = box.GetCorners();

	glm::vec3 newMin(FLT_MAX), newMax(-FLT_MAX);
	for (size_t i = 0; i < corners.size(); i++)
	{
		// Transform each corner
		glm::vec4 transformedCorner = transform * glm::vec4(corners[i], 1.0f);
		newMin = glm::min(newMin, glm::vec3(transformedCorner));
		newMax = glm::max(newMax, glm::vec3(transformedCorner));
	}
	return BoundingBox{ newMin, newMax };
}