#include "Engine/Renderer/Culling/Frustum.h"

Frustum::Frustum(const glm::mat4& m)
{
    auto extractPlane = [](Plane& p, const glm::vec4& row) {
        p.normal = glm::vec3(row.x, row.y, row.z);
        float invLen = 1.0f / glm::length(p.normal);
        p.normal *= invLen;
        p.d = row.w * invLen;
        };

    glm::vec4 row0 = glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]);
    glm::vec4 row1 = glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]);
    glm::vec4 row2 = glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]);
    glm::vec4 row3 = glm::vec4(m[0][3], m[1][3], m[2][3], m[3][3]);

    extractPlane(left, row3 + row0);
    extractPlane(right, row3 - row0);

    extractPlane(bottom, row3 + row1);
    extractPlane(top, row3 - row1);

    extractPlane(near, row3 + row2);
    extractPlane(far, row3 - row2);
}

bool AABBInFrustum(const Frustum& frustum, const BoundingBox& box)
{
    // Check each plane of the frustum
    const Plane* planes[6] = {
        &frustum.left,
        &frustum.right,
        &frustum.top,
        &frustum.bottom,
        &frustum.near,
        &frustum.far
    };
    for (const Plane* plane : planes)
    {
        // Compute the positive vertex (the vertex furthest in the direction of the plane normal)
        glm::vec3 pVertex = {
            plane->normal.x >= 0 ? box.max.x : box.min.x,
            plane->normal.y >= 0 ? box.max.y : box.min.y,
            plane->normal.z >= 0 ? box.max.z : box.min.z
        };
        // If the positive vertex is outside the plane, the AABB is outside the frustum
        if (glm::dot(plane->normal, pVertex) + plane->d < 0)
        {
            return false; // AABB is outside this plane
        }
    }
    return true; // AABB is inside or intersects the frustum
}
