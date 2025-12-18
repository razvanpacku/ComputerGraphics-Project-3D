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

	PrintHierarchy();
	childEntity1->Destroy();
	PrintHierarchy();
}

void TestScene::OnUpdate(double deltaTime)
{
	//std::cout << "test\n";
}