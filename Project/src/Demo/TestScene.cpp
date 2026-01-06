#include "Demo/TestScene.h"

#include "Engine/SceneGraph/Entities/Includes.h"
#include "Engine/SceneGraph/Systems/Includes.h"

#include <iostream>

void TestScene::OnCreate()
{
	Camera* camera = dynamic_cast<Camera*>(FindFirstChild("Camera"));
	if (camera) {
		camera->SetPosition({ 3.0f, 0.0f, 0.0f });
		camera->SetOrientation(180.0f, 0.0f);
	}
	Light* light = dynamic_cast<Light*>(FindFirstChild("Light"));
	if (light) {
		light->SetLightPosition({ 1.0f, 1.0f, 1.0f, 0.0f });
	}


	// Create a few entities for demonstration
	Entity* parentEntity = new Entity();
	Entity* childEntity1 = new Entity();
	Entity* childEntity2 = new Entity();
	Entity* grandChildEntity = new Entity();
	// Add entities to the scene graph
	AddOrMoveEntity(*parentEntity);
	AddOrMoveEntity(*childEntity1, parentEntity);
	AddOrMoveEntity(*childEntity2, parentEntity);
	AddOrMoveEntity(*grandChildEntity, childEntity1);

	// temp testing for transform system
	TransformEntity* transformEntity0 = new TransformEntity("Transform0");
	TransformEntity* transformEntity1 = new Anchor("Transform1");
	TransformEntity* transformEntity2 = new TransformEntity("Transform2");
	TransformEntity* transformEntity3 = new TransformEntity("Transform3");
	TransformEntity* transformEntity4 = new TransformEntity("Transform4");
	TransformEntity* transformEntity5 = new TransformEntity("Transform5");
	TransformEntity* transformEntity6 = new TransformEntity("Transform6");
	Entity* nonTransformEntity = new Entity();

	AddOrMoveEntity(*transformEntity0);
	AddOrMoveEntity(*transformEntity1, transformEntity0);
	AddOrMoveEntity(*transformEntity2, transformEntity0);
	AddOrMoveEntity(*transformEntity3, transformEntity1);
	AddOrMoveEntity(*nonTransformEntity, transformEntity2);
	AddOrMoveEntity(*transformEntity4, nonTransformEntity);
	AddOrMoveEntity(*transformEntity5, transformEntity4);
	AddOrMoveEntity(*transformEntity6, transformEntity4);

	transformEntity3->SetLocalPosition({ 1.0f, 0.0f, 0.0f });

	PrintHierarchy();
	childEntity1->Destroy();
	PrintHierarchy();
}

void TestScene::OnUpdate(double deltaTime)
{
	static double timeAccumulator = 0.0;
	timeAccumulator += deltaTime;

	// transform test orbit
	TransformEntity* transformEntity1 = dynamic_cast<TransformEntity*>(FindFirstDescendant("Transform1"));
	TransformEntity* transformEntity3 = dynamic_cast<TransformEntity*>(FindFirstDescendant("Transform3"));
	// rotate around y axis
	transformEntity1->SetLocalRotation(glm::angleAxis(static_cast<float>(timeAccumulator), glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::vec3 pos = transformEntity3->GetGlobalPosition();
	float distance = glm::length(pos);
	float angle = atan2(pos.z, pos.x);
	std::cout << "Transform3 Global Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ") " << distance << " " << angle << "\n";

	Light* light = dynamic_cast<Light*>(FindFirstChild("Light"));
	if (light) {
		//light->SetLightPosition({ sin(timeAccumulator), cos(timeAccumulator), 0.0f, 0.0f });
		light->SetLightPosition({ pos.x, pos.y, pos.z, 0.0f });
	}
}