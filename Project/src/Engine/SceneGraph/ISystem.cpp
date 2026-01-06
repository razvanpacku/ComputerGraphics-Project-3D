#include "Engine/SceneGraph/ISystem.h"

#include "Engine/SceneGraph/Scene.h"

ISystem::ISystem(Scene* scene, int16_t order)
	: scene(scene), order(order)
{
}
