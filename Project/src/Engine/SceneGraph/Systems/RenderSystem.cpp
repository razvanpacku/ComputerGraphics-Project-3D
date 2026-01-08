#include "Engine/SceneGraph/Systems/RenderSystem.h"
#include "Engine/Renderer/Renderer.h"

RenderSystem::RenderSystem(Scene* scene, int16_t order, Renderer* renderer, entt::registry* registry)
	: ISystem(scene, order), renderer(renderer), registry(registry)
{
	runOnStartup = true;

	auto* camera = dynamic_cast<Camera*>(scene->FindInternalEntity("Camera"));
	if (camera) {
		cameraComponent = &camera->GetComponent<CameraComponent>();
		renderer->SetRenderCamera(camera->renderCamera);
	}
	else {
		throw std::runtime_error("RenderSystem: No Camera entity found in the scene.");
	}
	auto* light = dynamic_cast<Light*>(scene->FindInternalEntity("Light"));
	if (light) {
		lightComponent = &light->GetComponent<LightComponent>();
		renderer->UpdateLighting(lightComponent);
	}
	else {
		throw std::runtime_error("RenderSystem: No Light entity found in the scene.");
	}
}

void RenderSystem::OnUpdate(double deltaTime)
{
	// check if camera should be updated
	if (cameraComponent->dirty) {
		auto* camera = dynamic_cast<Camera*>(scene->FindInternalEntity("Camera"));
		renderer->SetRenderCamera(camera->renderCamera);
		cameraComponent->dirty = false;
	}
	//check if light should be updated
	if(lightComponent->dirty) {
		renderer->UpdateLighting(lightComponent);
		lightComponent->dirty = false;
	}

	// Get RenderableComponents
	auto view = registry->view<RenderableComponent>();

	// Update each renderableComponent and submit to renderer
	for (auto entity : view)
	{
		auto& renderableC = view.get<RenderableComponent>(entity);
		renderableC.UpdateRenderables(deltaTime);
		renderer->Submit(renderableC.GetRenderables());
	}
}

void RenderSystem::UpdateTargetCamera(glm::vec3 targetPosition)
{
	auto* camera = dynamic_cast<Camera*>(scene->FindInternalEntity("Camera"));
	camera->SetPosition(targetPosition);
}