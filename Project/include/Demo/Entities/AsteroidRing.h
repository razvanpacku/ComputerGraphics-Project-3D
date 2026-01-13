#pragma once
#include "Engine/SceneGraph/Entities/TransformEntity.h"
#include "Engine/SceneGraph/Entities/RenderEntity.h"

struct AsteroidInstanceData
{
	float distance;
	float angle;
	float height;

	float scale;
};

// ================================================================
// AsteroidRing
//
// An entity that represents a ring of asteroids in the scene.
// ================================================================
class AsteroidRing : public TransformEntity, public RenderEntity
{
public:
	AsteroidRing(const std::string& name = "AsteroidRing");

	uint16_t GetAsteroidCount() const { return asteroidCount; }
	float GetInnerRadius() const { return innerRadius; }
	float GetOuterRadius() const { return outerRadius; }
	float GetVerticalSpread() const { return verticalSpread; }
	float GetMinScale() const { return minScale; }
	float GetMaxScale() const { return maxScale; }
	float GetRotationSpeed() const { return rotationSpeed; }

	void SetAsteroidCount(uint16_t count);
	void SetInnerRadius(float radius);
	void SetOuterRadius(float radius);
	void SetVerticalSpread(float spread);
	void SetMinScale(float scale);
	void SetMaxScale(float scale);
	void SetRotationSpeed(float speed) { rotationSpeed = speed; }
private:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateRenderables(double deltaTime, std::vector<Renderable>& renderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;

	std::vector<AsteroidInstanceData> asteroidLocals;

	uint16_t asteroidCount = 1000;

	float innerRadius = 5.0f;
	float outerRadius = 15.0f;

	float verticalSpread = 2.0f;

	float minScale = 0.1f;
	float maxScale = 0.3f;

	float rotationSpeed = 100.0f;
	
};

