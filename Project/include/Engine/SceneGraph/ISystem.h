#pragma once
#include <string>
class Scene;

// ======================================================
// ISystem
//
// Interface for systems that operate on the scene.
// ======================================================
class ISystem
{
public:
	ISystem(Scene* scene, int16_t order = 0);
	virtual ~ISystem() = default;
	virtual void OnUpdate(double deltaTime) = 0;

	virtual std::string GetName() const = 0;
	bool ShouldRunOnStartup() const { return runOnStartup; }
	int16_t GetOrder() const { return order; }
protected:
	template<typename T>
	T* const GetSystem() const;

	Scene* const scene;
	int16_t order;
	bool runOnStartup = false;
};

#include "Scene.h"

template<typename T>
T* const ISystem::GetSystem() const
{
	return scene->GetSystem<T>();
}
