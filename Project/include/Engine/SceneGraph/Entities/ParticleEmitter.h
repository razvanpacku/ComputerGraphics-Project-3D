#pragma once
#include "Engine/SceneGraph/Entities/TransformEntity.h"
#include "Engine/SceneGraph/Entities/RenderEntity.h"

struct ParticleInstanceData
{
	glm::vec3 position;     // local-space position
	glm::vec3 velocity;     // local-space velocity
	float lifetime;         // remaining time
	float maxLifetime;
	glm::mat4 spawnPointMatrix; // world matrix at spawn time
	bool detached = false; // whether the particle has detached from the emitter
};

// ================================================================
// ParticleEmitter
//
// An entity that emits particles.
// ================================================================
class ParticleEmitter : public TransformEntity, public RenderEntity
{
public:
	ParticleEmitter(const std::string& name = "AsteroidRing");

	bool GetState() const { return isEmitting; }
	float GetEmissionRate() const { return emissionRate; }
	float GetParticleLifetime() const { return particleLifetime; }
	float GetParticleSpeed() const { return particleSpeed; }
	float GetSpreadAngle() const { return spreadAngle; }
	float GetMaxScale() const { return maxScale; }
	glm::vec3 GetDirection() const { return direction; }

	void SetState(bool emitting) { isEmitting = emitting; }
	void SetEmissionRate(float rate) { emissionRate = rate; }
	void SetParticleLifetime(float lifetime) { particleLifetime = lifetime; }
	void SetParticleSpeed(float speed) { particleSpeed = speed; }
	void SetSpreadAngle(float angle) { spreadAngle = angle; }
	void SetMaxScale(float scale) { maxScale = scale; }
	void SetDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }
private:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateRenderables(double deltaTime, std::vector<Renderable>& renderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;

	void EmitParticle();

	std::vector<ParticleInstanceData> particles;

	Renderable particleTemplate;
	bool templateInitialized = false;

	bool isEmitting = true;

	float emissionRate = 50.0f;
	float particleLifetime = 4.0f;
	float particleSpeed = 4.0f;
	float spreadAngle = 30.0f;

	float maxScale = 1.0f;

	glm::vec3 direction = glm::vec3(0, 1, 0);

	float emissionAccumulator = 0.0f;

};

