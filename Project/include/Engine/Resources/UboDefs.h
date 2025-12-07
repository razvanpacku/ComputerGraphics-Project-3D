#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

#define STD140 alignas(16)

#define FIXED_VEC3 glm::aligned_vec4

namespace glm {
	class fixed_vec3 : public FIXED_VEC3 {
	public:
		fixed_vec3() : FIXED_VEC3() {}
		fixed_vec3(float x, float y, float z) : FIXED_VEC3(x, y, z, 0.0f) {}
		fixed_vec3(const glm::vec3& v) : FIXED_VEC3(v.x, v.y, v.z, 0.0f) {}
	};
}

inline glm::fixed_vec3 fixedVec3(float x, float y, float z) {
	return glm::fixed_vec3(x, y, z);
}
inline glm::fixed_vec3 fixedVec3(const glm::vec3& v) {
	return glm::fixed_vec3(v);
}

// UBO struct definitions

struct STD140 MatricesUBO {
	glm::aligned_mat4 model;
	glm::aligned_mat4 view;
	glm::aligned_mat4 projection;
};

struct STD140 LightingUBO
{
	glm::aligned_vec4 lightPos;
	glm::fixed_vec3 lightColor;
	float ambientStrength;
	glm::fixed_vec3 attenuationFactor;
};

struct STD140 MaterialUBO {
	float shininess;
	float specularStrength;
	float metalicity;
};

struct STD140 CameraUBO
{
	glm::fixed_vec3 viewPos;
};