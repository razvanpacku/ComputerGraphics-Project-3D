#include "Demo/Entities/AsteroidRing.h"

#include "Engine/Renderer/RenderableProvider/ModelRenderableProvider.h"

glm::mat4 asteroidMatrix(const AsteroidInstanceData& data) {
	glm::mat4 M(1.0f);
	M = glm::rotate(M, glm::radians(data.angle), glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::translate(M, glm::vec3(data.distance, data.height, 0.0f));
	M = glm::scale(M, glm::vec3(data.scale));

	return M;
}

// ================================================================
// AsteroidRing
// ================================================================
AsteroidRing::AsteroidRing(const std::string& name)
	: Entity(name), RenderEntity(name), TransformEntity(name)
{
	renderableProvider = new ModelRenderableProvider();
}

void AsteroidRing::SetAsteroidCount(uint16_t count)
{
	asteroidCount = count;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::SetInnerRadius(float radius)
{
	innerRadius = radius;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::SetOuterRadius(float radius)
{
	outerRadius = radius;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::SetVerticalSpread(float spread)
{
	verticalSpread = spread;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::SetMinScale(float scale)
{
	minScale = scale;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::SetMaxScale(float scale)
{
	maxScale = scale;
	renderableComponent->isGenerated = false;
}

void AsteroidRing::ProvideRenderables(std::vector<Renderable>& outRenderables)
{
	auto* modelProvider = dynamic_cast<ModelRenderableProvider*>(renderableProvider);
	auto& models = ResourceManager::Get().models;
	modelProvider->model = models.Get("asteroid");
	modelProvider->GenerateRenderables(outRenderables);

	Renderable r = outRenderables[0];
	outRenderables.clear();
	asteroidLocals.clear();

	glm::mat4 entityMatrix = GetComponent<TransformComponent>().worldMatrix;

	for(size_t i = 0; i < asteroidCount; i++) {

		float angle = (float)(rand() % 360);
		float distance = innerRadius + (float)(rand() % 1000) / 1000.0f * (outerRadius - innerRadius);
		float height = (((float)(rand() % 200) / 200.0f) - 0.5f) * verticalSpread;
		float scale = minScale + (float)(rand() % 500) / 500.0f * (maxScale - minScale);

		asteroidLocals.push_back({ distance, angle, height, scale });

		r.modelMatrix = entityMatrix * asteroidMatrix(asteroidLocals.back());

		outRenderables.push_back(r);
	}
}

void AsteroidRing::UpdateRenderables(double deltaTime, std::vector<Renderable>& renderables)
{
	glm::mat4 entityMatrix = GetComponent<TransformComponent>().worldMatrix;
	for(size_t i = 0; i < asteroidLocals.size(); i++) {
		asteroidLocals[i].angle += rotationSpeed * static_cast<float>(deltaTime) / (asteroidLocals[i].distance * asteroidLocals[i].distance);
		if(asteroidLocals[i].angle > 360.0f) {
			asteroidLocals[i].angle -= 360.0f;
		}
		renderables[i].modelMatrix = entityMatrix * asteroidMatrix(asteroidLocals[i]);
	}
}

void AsteroidRing::UpdateTransform(const glm::mat4& newTransform) {
	glm::mat4 entityMatrix = newTransform;
	for(size_t i = 0; i < asteroidLocals.size(); i++) {
		renderableComponent->renderables[i].modelMatrix = entityMatrix * asteroidMatrix(asteroidLocals[i]);
	}
}
