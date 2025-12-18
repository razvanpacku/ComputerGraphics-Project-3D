#pragma once
#include <glad/glad.h>

#include "Engine/Resources/ResourceManager.h"
#include "RenderQueue.h"
#include "GLStateCache.h"
#include "ShadowFramebuffer.h"
#include "Engine/Resources/UboDefs.h"

#include "IRenderCamera.h"
#include "Engine/Components/LightComponent.h"

#define DEFAULT_CLEAR_COLOR_R 0.0f
#define DEFAULT_CLEAR_COLOR_G 0.0f
#define DEFAULT_CLEAR_COLOR_B 0.0f

#define SHADOW_MAP_SIZE 2048
#define POINT_SHADOW_TEX_NAME "shadow/point"
#define DIR_SHADOW_TEX_NAME "shadow/dir"

//forward declarations
class App;

class Renderer
{
public:
	Renderer(App& app);
	~Renderer();

	void Render();

	// =================================================
	void Submit(const Renderable& r) { renderQueue.Push(r); }
	void Submit(const std::vector<Renderable>& rs) { renderQueue.Push(rs); }

	void SetRenderCamera(IRenderCamera* camera) { renderCamera = camera; }
	void UpdateLighting(LightingUBO* light = nullptr);
private:
	
	App& app;
	ResourceManager& _rm;

	RenderQueue renderQueue;
	GLStateCache glState;

	IRenderCamera* renderCamera = nullptr;
	LightingUBO* renderLight = nullptr;

	ShadowFramebuffer
		pointShadowFBO	= ShadowFramebuffer(ShadowMapType::Point),
		dirShadowFBO	= ShadowFramebuffer(ShadowMapType::Directional);

	bool showBoundingBoxes = false;

	// cascaded shadow mapping related variables
	float nearPlane = 0.1f, farPlane = 10000.f;
	fixed_float cascadeSplits[6];


	// --- Rendering functions ---
	void Clear() const;
	void UpdateCameraUBOs();
	void GetRenderables();
	void RenderFrame();
	void ClearQueue() { renderQueue.Clear(); }

	// -- Draw passes ---
	void DrawShadowPass();
	void DrawMainPass();

	// --- Drawing functions ---
	void DrawList(const std::vector<RenderSubmission>& submissions);
	void DrawSubmission(const RenderSubmission& submission);
	void DrawShadowSubmission(const RenderSubmission& submission);
	void DrawGUISubmission(const RenderSubmission& submission);

	// --- Util camera functions ---
	glm::mat4 GetViewMatrix() const {
		return renderCamera->GetViewMatrix();
	}
	glm::mat4 GetPerspectiveMatrix() const {
		return renderCamera->GetProjectionMatrix();
	}
	glm::mat4 GetGUIViewMatrix() const {
		return glm::ortho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
	}

	// Temporary functions
	void Initialize();
	void Cleanup();
};

