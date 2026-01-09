#include "Demo/Entities/Rocket.h"
#include "Engine/SceneGraph/Systems/CollisionSystem.h"

#include "Engine/InputManager.h"
#include <GLFW/glfw3.h>

Rocket::Rocket(const std::string& name)
	: Entity(name), ModelEntity("rocket", name)
{
	SetLocalScale(glm::vec3(0.2f));
	auto* col = GetSystem<CollisionSystem>();
	col->GiveCollisionShape(this, { RigidBodyShape::Cylinder, 1.5f, 11.f }, 1.f);
	thrusterParticles = new ParticleEmitter("ParticleEmitter");
	thrusterParticles->SetParent(this);
	thrusterParticles->SetLocalRotation(glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)));
	thrusterParticles->SetLocalPosition({ 0.0f, -5.f, 0.0f });
	thrusterParticles->SetState(false);

	auto& _im = InputManager::Get();

	connections.emplace_back(_im.BindKey(GLFW_KEY_SPACE, InputEventType::Pressed, [this]() {
		this->thrusterParticles->SetState(true);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_SPACE, InputEventType::Released, [this]() {
		this->thrusterParticles->SetState(false);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_SPACE, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 impulse = rot * glm::vec3(0.0f, 0.1f, 0.0f) * static_cast<float>(this->deltaTime);

		rc.ApplyImpulse(impulse);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_W, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 0.0f, 1.0f) * static_cast<float>(this->deltaTime) * 1000.f;
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_S, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 0.0f, -1.0f) * 1.f * static_cast<float>(this->deltaTime);;
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_A, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(1.0f, 0.0f, 0.0f) * 1.f * static_cast<float>(this->deltaTime);
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_D, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(-1.0f, 0.0f, 0.0f) * 1.f * static_cast<float>(this->deltaTime);
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_Q, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, -1.0f, 0.0f) * 0.1f * static_cast<float>(this->deltaTime);
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_E, InputEventType::Held, [this]() {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		glm::quat rot = this->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f * static_cast<float>(this->deltaTime);
		rc.AddTorque(torque);
		}));

	connections.emplace_back(_im.BindKey(GLFW_KEY_R, InputEventType::Pressed, [this]() {
		this->isStabilizing = !this->isStabilizing;
		}));
}

void Rocket::Update(double deltaTime)
{
	this->deltaTime = deltaTime;
	if (isStabilizing) {
		auto& rc = this->GetComponent<RigidBodyComponent>();
		rc.angularVelocity *= 0.99f;
	}
}