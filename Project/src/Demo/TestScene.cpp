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

	AsteroidRing* asteroidRing = new AsteroidRing("AsteroidRing");
	asteroidRing->SetInnerRadius(350.f);
	asteroidRing->SetOuterRadius(400.f);
	AddOrMoveEntity(*asteroidRing);

	auto* col = GetSystem<CollisionSystem>();

	Anchor* planetAnchor = new Anchor("PlanetAnchor");
	AddOrMoveEntity(*planetAnchor);

	BasePart* planet = new BasePart(BasePartShape::SPHERE, "plastic", "Planet");
	col->GiveCollisionShape(planet, { }, 500.0f);
	//col->GiveCollisionShape(ball2, { RigidBodyShape::Sphere, 1.5f, 0.0f});
	AddOrMoveEntity(*planet, planetAnchor);
	planet->SetGlobalScale(glm::vec3(20.0f));

	Anchor* moonAnchor = new Anchor("MoonAnchor");
	AddOrMoveEntity(*moonAnchor, planetAnchor);

	BasePart* moon = new BasePart(BasePartShape::SPHERE, "plastic", "Moon");
	col->GiveCollisionShape(moon, {}, 400.f);
	AddOrMoveEntity(*moon, moonAnchor);
	moon->SetGlobalScale(glm::vec3(15.0f));
	moon->SetLocalPosition({ 300.0f, 0.0f, 0.0f });

	ModelEntity* rocketCollide = new ModelEntity("rocket", "RocketCollide");
	col->GiveCollisionShape(rocketCollide, { RigidBodyShape::Cylinder, 1.5f, 11.f }, 1.f);
	AddOrMoveEntity(*rocketCollide);
	rocketCollide->SetLocalScale(glm::vec3(0.2f));
	rocketCollide->SetGlobalPosition({ 0.0f, 0.0f, -11.0f });
	rocketCollide->SetGlobalRotation(glm::quat(glm::vec3(glm::radians(-90.f), 0.0f, 0.0f)));

	ParticleEmitter* particleEmitter2 = new ParticleEmitter("ParticleEmitter2");
	AddOrMoveEntity(*particleEmitter2, rocketCollide);
	particleEmitter2->SetLocalRotation(glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)));
	particleEmitter2->SetLocalPosition({ 0.0f, -5.f, 0.0f });
	particleEmitter2->SetState(false);

	auto& rc2 = planet->GetComponent<RigidBodyComponent>();
	rc2.anchored = true;

	auto& rc = moon->GetComponent<RigidBodyComponent>();
	rc.anchored = true;

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

	Textbox* fpsText = dynamic_cast<Textbox*>(FindFirstDescendant("FpsText"));
	int fps = static_cast<int>(1.0f / deltaTime);
	fpsText->SetText("FPS: " + std::to_string(fps));

	BasePart* planet = dynamic_cast<BasePart*>(FindFirstDescendant("Planet"));
	Anchor* moonAnchor = dynamic_cast<Anchor*>(FindFirstDescendant("MoonAnchor"));
	BasePart* moon = dynamic_cast<BasePart*>(FindFirstDescendant("Moon"));

	moonAnchor->SetLocalRotation(glm::quat(glm::vec3(0.0f, static_cast<float>(timeAccumulator) * glm::radians(0.02f), 0.0f)));
	moon->SetLocalRotation(glm::quat(glm::vec3(0.0f, static_cast<float>(timeAccumulator) * glm::radians(-0.02f), 0.0f)));

	TransformEntity* rocketCollide = dynamic_cast<TransformEntity*>(FindFirstDescendant("RocketCollide"));
	auto& rc1 = rocketCollide->GetComponent<RigidBodyComponent>();
	auto& rc2 = planet->GetComponent<RigidBodyComponent>();
	auto& rc3 = moon->GetComponent<RigidBodyComponent>();
	glm::vec3 dir = glm::vec3(planet->GetGlobalPosition() - rocketCollide->GetGlobalPosition());
	float distance = glm::length(dir);
	glm::vec3 force = (glm::normalize(dir) * (rc1.mass * rc2.mass)/ (distance * distance)) * 1.f;
	rc1.AddForce(force);
	glm::vec3 dir2 = glm::vec3(moon->GetGlobalPosition() - rocketCollide->GetGlobalPosition());
	distance = glm::length(dir2);
	force = (glm::normalize(dir2) * (rc1.mass * rc3.mass) / (distance * distance)) * 1.f;
	rc1.AddForce(force);
	//rc2.AddForce(-force);
}