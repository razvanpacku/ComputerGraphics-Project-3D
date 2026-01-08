#include "Demo/TestScene.h"

#include "Engine/SceneGraph/Entities/Includes.h"
#include "Engine/SceneGraph/Systems/Includes.h"

#include "Demo/Entities/AsteroidRing.h"

#include "Engine/InputManager.h"
#include <GLFW/glfw3.h>

#include <iostream>

void TestScene::OnCreate()
{
	Camera* camera = dynamic_cast<Camera*>(FindFirstChild("Camera"));
	if (camera) {
		camera->SetPosition({ 3.0f, 0.0f, 0.0f });
		camera->SetOrientation(180.0f, 0.0f);

		camera->SwitchCameraType(CameraType::ORBIT_CAMERA);
	}
	Light* light = dynamic_cast<Light*>(FindFirstChild("Light"));
	if (light) {
		light->SetLightPosition({ 1.0f, 1.0f, 1.0f, 0.0f });
	}

	Textbox* fpsText = new Textbox("FPS: ", "FpsText");
	fpsText->SetRelativeScale({ 0.f,0.f });
	fpsText->SetAbsoluteScaleOffset({ 128.f,16.f });
	fpsText->SetRelativePosition({ 0.f, 1.f });
	fpsText->SetAnchorPoint({ 0.f, 1.f });
	AddOrMoveEntity(*fpsText);

	// entities to replicate the original test scene
	ModelEntity* rocket = new ModelEntity("rocket", "Rocket");
	rocket->SetLocalScale(glm::vec3(0.2f));
	AddOrMoveEntity(*rocket);

	ParticleEmitter* particleEmitter = new ParticleEmitter("ParticleEmitter");
	AddOrMoveEntity(*particleEmitter, rocket);
	particleEmitter->SetLocalRotation(glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)));
	particleEmitter->SetLocalPosition({ 0.0f, -5.f, 0.0f });

	BasePart* quad = new BasePart(BasePartShape::QUAD, "matte", "Quad");
	quad->SetLocalPosition({ -20.0f, 0.0f, 0.0f });
	quad->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)));
	quad->SetLocalScale({ 100.0f, 100.0f, 1.0f });
	AddOrMoveEntity(*quad);

	AsteroidRing* asteroidRing = new AsteroidRing("AsteroidRing");
	asteroidRing->SetAsteroidCount(1000);
	AddOrMoveEntity(*asteroidRing);

	auto* col = GetSystem<CollisionSystem>();

	BasePart* ball1 = new BasePart(BasePartShape::SPHERE, "plastic", "Ball1");
	col->GiveCollisionShape(ball1, {});
	AddOrMoveEntity(*ball1);
	ball1->SetGlobalPosition({ 0.f, 1.5f, 1.0f });

	BasePart* ball2 = new BasePart(BasePartShape::SPHERE, "plastic", "Ball2");
	col->GiveCollisionShape(ball2, { });
	//col->GiveCollisionShape(ball2, { RigidBodyShape::Sphere, 1.5f, 0.0f});
	AddOrMoveEntity(*ball2);
	ball2->SetGlobalPosition({ 0.0f, -10.0f, -2.5f });
	ball2->SetGlobalScale(glm::vec3(7.0f));

	ModelEntity* rocketCollide = new ModelEntity("rocket", "RocketCollide");
	col->GiveCollisionShape(rocketCollide, { RigidBodyShape::Cylinder, 1.5f, 11.f }, 1.f);
	AddOrMoveEntity(*rocketCollide);
	rocketCollide->SetLocalScale(glm::vec3(0.2f));
	rocketCollide->SetGlobalPosition({ 0.0f, 0.0f, -2.5f });

	ParticleEmitter* particleEmitter2 = new ParticleEmitter("ParticleEmitter2");
	AddOrMoveEntity(*particleEmitter2, rocketCollide);
	particleEmitter2->SetLocalRotation(glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)));
	particleEmitter2->SetLocalPosition({ 0.0f, -5.f, 0.0f });
	particleEmitter2->SetState(false);

	auto& rc = ball1->GetComponent<RigidBodyComponent>();
	rc.ApplyImpulse(glm::vec3(0.5f, 0.0f, -0.2f));

	auto& rc2 = ball2->GetComponent<RigidBodyComponent>();
	//rc2.anchored = true;

	auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
	//rc3.ApplyImpulse(glm::vec3(-0.5f, 0.0f, 0.2f));
	//rc3.angularVelocity = glm::vec3(0.0f, 10.f, 0.0f);

	camera->SetTarget(rocketCollide);

	auto& _im = InputManager::Get();

	_im.BindKey(GLFW_KEY_SPACE, InputEventType::Pressed, [particleEmitter2]() {
		particleEmitter2->SetState(true);
		});

	_im.BindKey(GLFW_KEY_SPACE, InputEventType::Released, [particleEmitter2]() {
		particleEmitter2->SetState(false);
		});

	_im.BindKey(GLFW_KEY_SPACE, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 impulse = rot * glm::vec3(0.0f, 0.1f, 0.0f);

		rc3.ApplyImpulse(impulse);
		});

	_im.BindKey(GLFW_KEY_W, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 0.0f, 1.0f) * 1.f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_S, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 0.0f, -1.0f) * 1.f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_A, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(1.0f, 0.0f, 0.0f) * 1.f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_D, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(-1.0f, 0.0f, 0.0f) * 1.f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_Q, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, -1.0f, 0.0f) * 0.1f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_E, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		glm::quat rot = rocketCollide->GetGlobalRotation();
		glm::vec3 torque = rot * glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f;
		rc3.AddTorque(torque);
		});

	_im.BindKey(GLFW_KEY_R, InputEventType::Held, [rocketCollide]() {
		auto& rc3 = rocketCollide->GetComponent<RigidBodyComponent>();
		rc3.angularVelocity *= 0.99f;
		});

	PrintHierarchy();
}

void TestScene::OnUpdate(double deltaTime)
{
	static double timeAccumulator = 0.0;
	timeAccumulator += deltaTime;

	ModelEntity* rocket = dynamic_cast<ModelEntity*>(FindFirstDescendant("Rocket"));
	if (rocket) {
		float angle = static_cast<float>(timeAccumulator) * glm::radians(20.0f);
		rocket->SetLocalPosition({ 4.0f * cos(angle), 0.0f, 4.0f * sin(angle) });

		glm::vec3 direction = glm::normalize(glm::vec3(-4.0f * sin(angle), 0.0f, 4.0f * cos(angle)));
		if (glm::length(direction) > 1e-6f) {
			// rotate local +Y onto movement direction
			glm::quat targetRotation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), direction);
			rocket->SetLocalRotation(targetRotation);
		}
	}

	Textbox* fpsText = dynamic_cast<Textbox*>(FindFirstDescendant("FpsText"));
	int fps = static_cast<int>(1.0f / deltaTime);
	fpsText->SetText("FPS: " + std::to_string(fps));

	/*
	BasePart* ball = dynamic_cast<BasePart*>(FindFirstDescendant("Ball"));
	auto& rc = ball->GetComponent<RigidBodyComponent>();

	glm::vec3 dir = glm::vec3(rocket->GetGlobalPosition() - ball->GetGlobalPosition());
	float distance = glm::length(dir);
	glm::vec3 force = (glm::normalize(dir) / (distance * distance)) * 10.f;

	rc.AddForce(force);
	rc.ApplyImpulse(-rc.velocity * 0.01f);
	*/

	//BasePart* ball1 = dynamic_cast<BasePart*>(FindFirstDescendant("Ball1"));
	ModelEntity* rocketCollide = dynamic_cast<ModelEntity*>(FindFirstDescendant("RocketCollide"));
	BasePart* ball2 = dynamic_cast<BasePart*>(FindFirstDescendant("Ball2"));
	auto& rc1 = rocketCollide->GetComponent<RigidBodyComponent>();
	auto& rc2 = ball2->GetComponent<RigidBodyComponent>();
	glm::vec3 dir = glm::vec3(ball2->GetGlobalPosition() - rocketCollide->GetGlobalPosition());
	float distance = glm::length(dir);
	if (distance > 0.25f) {
		glm::vec3 force = (glm::normalize(dir) / (distance * distance)) * 30.f;
		rc1.AddForce(force);
		rc2.AddForce(-force);
	}
}