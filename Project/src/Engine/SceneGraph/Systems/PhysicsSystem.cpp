#include "Engine/SceneGraph/Systems/PhysicsSystem.h"

#include "Engine/SceneGraph/Systems/TransformSystem.h"
#include "Engine/SceneGraph/Entities/TransformEntity.h"

// ======================================================
// PhysicsSystem
// ======================================================

PhysicsSystem::PhysicsSystem(Scene* scene, int16_t order, entt::registry* registry)
	: ISystem(scene, order), registry(registry)
{
	runOnStartup = true;
}

void PhysicsSystem::OnUpdate(double deltaTime)
{
	auto view = registry->view<TransformComponent, RigidBodyComponent>();
	for (auto& entity : view) {
		auto& rb = view.get<RigidBodyComponent>(entity);
		auto& tf = view.get<TransformComponent>(entity);

		if (rb.anchored)
			continue;

		// make sure transform is up to date for global rotation
		auto* transformSystem = GetSystem<TransformSystem>();
		transformSystem->UpdateEntity(entity);
		glm::quat q = TransformFunctions::DecomposeRotation(tf.worldMatrix);
		glm::mat3 R = glm::mat3_cast(q);

		// rotate inertia into world
		rb.inertiaTensorWorld = R * rb.inertiaTensor * glm::transpose(R);
		rb.inverseInertiaTensorWorld = glm::inverse(rb.inertiaTensorWorld);

		// compute acceleration
		glm::vec3 acceleration = rb.forceAccum / rb.mass;
		glm::vec3 angularAcceleration = rb.inverseInertiaTensorWorld * rb.torqueAccum;

		// integrate velocity and position
		// semi-implicit Euler integration
		rb.velocity += acceleration * static_cast<float>(deltaTime);
		rb.angularVelocity += angularAcceleration * static_cast<float>(deltaTime);

		tf.position += rb.velocity * static_cast<float>(deltaTime);
		tf.rotation = glm::normalize(
			tf.rotation + glm::quat(0.0f, rb.angularVelocity * static_cast<float>(deltaTime)) * tf.rotation * 0.5f
		);

		tf.localDirty = true;
		transformSystem->MarkDirty(entity);

		// clear accumulators
		rb.forceAccum = glm::vec3(0.0f);
		rb.torqueAccum = glm::vec3(0.0f);
	}
}

void PhysicsSystem::GiveRigidBody(Entity* entity, const RigidBodyInitData& rigidBodyData, float mass, bool anchored) {
	auto& entHandle = entity->GetHandle();
	auto& rb = registry->emplace<RigidBodyComponent>(entHandle);

	// get global scale from TransformComponent
	float scale = 1.f;
	TransformEntity* transfEnt = dynamic_cast<TransformEntity*>(entity);
	if (transfEnt) {
		glm::vec3 nonUniformScale = transfEnt->GetGlobalScale();
		// we aproximmate non uniform scale
		scale = glm::length(nonUniformScale) / sqrt(3.0f);
	}

	rb.mass = mass;

	float r = rigidBodyData.radius;
	float h = rigidBodyData.height;

	// inertia tensor computation
	switch (rigidBodyData.shape) {
	case RigidBodyShape::Sphere: {
		float v = (2.0f / 5.0f) * mass * r * r;
		rb.inertiaTensor = glm::mat3(v);
		break;
	}
	case RigidBodyShape::Cylinder: {
		//glm::vec3 axis = glm::normalize(rigidBodyData.orientationAxis);
		// Cylinder inertia aligned to local +Y
		// Iyy = 1/2 m r^2
		// Ixx = Izz = 1/12 m (3r^2 + h^2)
		float Iyy = 0.5f * mass * (r * r);
		float Ixx = (1.0f / 12.0f) * mass * (3.0f * r * r + h * h);
		float Izz = Ixx;

		glm::mat3 I_body(0.0f);
		I_body[0][0] = Ixx;
		I_body[1][1] = Iyy;
		I_body[2][2] = Izz;
		/*
		// If axis already (0,1,0), use directly
		if (glm::all(glm::epsilonEqual(axis, glm::vec3(0, 1, 0), 1e-4f))) {
			rb.inertiaTensor = I_body;
		}
		else {
			// Build rotation matrix that rotates +Y to given axis
			glm::vec3 up = glm::vec3(0, 1, 0);
			glm::vec3 v = glm::cross(up, axis);
			float c = glm::dot(up, axis);
			float k = 1.0f / (1.0f + c);

			// If axis is opposite up, use 180° flip
			if (glm::length(v) < 1e-6f) {
				glm::mat3 R = glm::mat3(-1.0f);
				R[1][1] = 1.0f;
				rb.inertiaTensor = R * I_body * glm::transpose(R);
			}
			else {
				// Rodrigues rotation building a matrix-frame
				glm::mat3 vx = glm::mat3(0.0f);
				vx[0][1] = -v.z; vx[0][2] = v.y;
				vx[1][0] = v.z; vx[1][2] = -v.x;
				vx[2][0] = -v.y; vx[2][1] = v.x;

				glm::mat3 R = glm::mat3(1.0f) + vx + vx * vx * k;
				rb.inertiaTensor = R * I_body * glm::transpose(R);
			}
		}
		*/
		rb.inertiaTensor = I_body; // aligned to local +Y for simplicity
		break;
	}
	}
	rb.inertiaTensor = scale * scale * rb.inertiaTensor;
	rb.inverseInertiaTensor = glm::inverse(rb.inertiaTensor);
}
