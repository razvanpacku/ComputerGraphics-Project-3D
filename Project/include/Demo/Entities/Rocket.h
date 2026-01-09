#pragma once
#include "Engine/InputManager.h"
#include "Engine/SceneGraph/Entities/ModelEntity.h"
#include "Engine/SceneGraph/Entities/ParticleEmitter.h"


class Rocket : public ModelEntity, public ConnectionHolder
{
public:
	Rocket(const std::string& name = "Rocket");
	float GetFuel() const { return fuel; }
	float GetCharge() const { return charge; }
	bool IsStabilizing() const { return isStabilizing; }
	void Update(double deltaTime);
private:
	float fuel = 10.0f;
	float charge = 10.0f;
	bool isStabilizing = false;
	double deltaTime = 0.0;
	ParticleEmitter* thrusterParticles;
};