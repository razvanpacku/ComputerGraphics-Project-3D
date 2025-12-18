#include "Engine/SceneGraph/ISystem.h"

#include "Engine/SceneGraph/Scene.h"

ISystem::ISystem(Scene* scene)
	: scene(scene)
{
}
