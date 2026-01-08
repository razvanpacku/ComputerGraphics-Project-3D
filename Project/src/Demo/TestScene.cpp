#include "Demo/TestScene.h"

#include "Engine/SceneGraph/Entities/Includes.h"
#include "Engine/SceneGraph/Systems/Includes.h"

#include "Demo/Entities/AsteroidRing.h"

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
	camera->SetTarget(rocket);

	ParticleEmitter* particleEmitter = new ParticleEmitter("ParticleEmitter");
	AddOrMoveEntity(*particleEmitter, rocket);
	particleEmitter->SetLocalRotation(glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)));
	particleEmitter->SetLocalPosition({ 0.0f, -1.0f, 0.0f });

	BasePart* quad = new BasePart(BasePartShape::QUAD, "matte", "Quad");
	quad->SetLocalPosition({ -20.0f, 0.0f, 0.0f });
	quad->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)));
	quad->SetLocalScale({ 100.0f, 100.0f, 1.0f });
	AddOrMoveEntity(*quad);

	AsteroidRing* asteroidRing = new AsteroidRing("AsteroidRing");
	asteroidRing->SetAsteroidCount(1000);
	AddOrMoveEntity(*asteroidRing);

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
}