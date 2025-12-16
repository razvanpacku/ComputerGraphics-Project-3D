#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

#define STD140 alignas(16)

// =========================================
// fixed_vec3
//
// A 3-component vector aligned to 16 bytes for UBO compatibility.
// =========================================
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

// =========================================
// fixed_float
//
// A float aligned to 16 bytes for UBO compatibility. Use this when you need an array of floats.
// =========================================

struct fixed_float
{
public:
	STD140 float value;
private:
	float _padding[3];
public:
    // Constructors
    fixed_float() : value(0.0f) {}
    fixed_float(float v) : value(v) {}

    // Implicit conversion to float
    operator float() const { return value; }

    // Assignment from float
    fixed_float& operator=(float v) { value = v; return *this; }

    // Unary operators
    fixed_float operator+() const { return fixed_float(+value); }
    fixed_float operator-() const { return fixed_float(-value); }

    // Arithmetic operators
    fixed_float operator+(const fixed_float& rhs) const { return fixed_float(value + rhs.value); }
    fixed_float operator-(const fixed_float& rhs) const { return fixed_float(value - rhs.value); }
    fixed_float operator*(const fixed_float& rhs) const { return fixed_float(value * rhs.value); }
    fixed_float operator/(const fixed_float& rhs) const { return fixed_float(value / rhs.value); }

    fixed_float& operator+=(const fixed_float& rhs) { value += rhs.value; return *this; }
    fixed_float& operator-=(const fixed_float& rhs) { value -= rhs.value; return *this; }
    fixed_float& operator*=(const fixed_float& rhs) { value *= rhs.value; return *this; }
    fixed_float& operator/=(const fixed_float& rhs) { value /= rhs.value; return *this; }

    // Arithmetic with float
    fixed_float operator+(float rhs) const { return fixed_float(value + rhs); }
    fixed_float operator-(float rhs) const { return fixed_float(value - rhs); }
    fixed_float operator*(float rhs) const { return fixed_float(value * rhs); }
    fixed_float operator/(float rhs) const { return fixed_float(value / rhs); }

    fixed_float& operator+=(float rhs) { value += rhs; return *this; }
    fixed_float& operator-=(float rhs) { value -= rhs; return *this; }
    fixed_float& operator*=(float rhs) { value *= rhs; return *this; }
    fixed_float& operator/=(float rhs) { value /= rhs; return *this; }

    // Comparison operators
    bool operator==(const fixed_float& rhs) const { return value == rhs.value; }
    bool operator!=(const fixed_float& rhs) const { return value != rhs.value; }
    bool operator< (const fixed_float& rhs) const { return value < rhs.value; }
    bool operator<=(const fixed_float& rhs) const { return value <= rhs.value; }
    bool operator> (const fixed_float& rhs) const { return value > rhs.value; }
    bool operator>=(const fixed_float& rhs) const { return value >= rhs.value; }

    bool operator==(float rhs) const { return value == rhs; }
    bool operator!=(float rhs) const { return value != rhs; }
    bool operator< (float rhs) const { return value < rhs; }
    bool operator<=(float rhs) const { return value <= rhs; }
    bool operator> (float rhs) const { return value > rhs; }
    bool operator>=(float rhs) const { return value >= rhs; }

    // Optional: swap with float
    void set(float v) { value = v; }
    float get() const { return value; }
};

// --- UBO Definitions ---

struct STD140 LightingUBO
{
	glm::aligned_vec4 lightPos;
	glm::fixed_vec3 lightColor;
	float ambientStrength;
	glm::fixed_vec3 attenuationFactor;
};

// A Light struct with default values, used by Renderer
struct Light : public LightingUBO {
    Light() {
        lightPos = glm::aligned_vec4(0.0f);
        lightColor = glm::fixed_vec3(1.0f, 1.0f, 1.0f);
        ambientStrength = 0.1f;
        attenuationFactor = glm::fixed_vec3(1.0f, 0.09f, 0.032f);
    }
};

struct STD140 ShadowUBO
{
	glm::aligned_mat4 lightSpaceMatrix[6];
	glm::aligned_vec4 lightPos;
    fixed_float cascadedSplits[6];
};

struct STD140 MaterialUBO {
	float shininess;
	float specularStrength;
	float metalicity;
};

struct STD140 CameraUBO
{
	glm::fixed_vec3 viewPos;
	glm::aligned_mat4 view;
	glm::aligned_mat4 projection;
};

struct STD140 GUICameraUBO
{
    glm::aligned_mat4 view;
};