#pragma once

#include <glm/glm.hpp>

enum class RigidBodyShape
{
	Sphere,
	Cylinder,
};

// for defining the shape of the rigid body
struct RigidBodyInitData {
	RigidBodyShape shape = RigidBodyShape::Sphere;
	float radius = 0.5f;
	float height = 1.0f;
};

struct RigidBodyComponent
{
	glm::vec3 velocity = glm::vec3(0.0f); // Linear velocity
	glm::vec3 angularVelocity = glm::vec3(0.0f); // Angular velocity (in radians)

	glm::vec3 forceAccum = glm::vec3(0.0f); // Accumulated forces (for impulse integration)
	glm::vec3 torqueAccum = glm::vec3(0.0f); // Accumulated torques

	float mass = 1.0f; // Mass of the rigid body

	glm::mat3 inertiaTensor = glm::mat3(1.0f); // Inertia tensor for rotational dynamics
	glm::mat3 inverseInertiaTensor = glm::mat3(1.0f); // Inverse of the inertia tensor, precomputed for efficiency

	glm::mat3  inertiaTensorWorld = glm::mat3(1.0f);        // rotated into world
	glm::mat3  inverseInertiaTensorWorld = glm::mat3(1.0f); // inverse(world)

	bool anchored = false; // If true, the rigid body is static and is not affected by forces (still influences other bodies for collisions and other things);
	RigidBodyComponent() = default;

	void AddForce(const glm::vec3& F) {
		forceAccum += F;
	}
	void AddForceAtPoint(const glm::vec3& F,
		const glm::vec3& offset) {
		forceAccum += F;
		torqueAccum += glm::cross(offset, F);
	}
	void ApplyImpulse(const glm::vec3& impulse,
		const glm::vec3& contactOffset = glm::vec3(0.0f)) {
		velocity += impulse / mass;
		angularVelocity += inverseInertiaTensor * glm::cross(contactOffset, impulse);
	}
	void AddTorque(const glm::vec3& T) {
		torqueAccum += T;
	}
};
