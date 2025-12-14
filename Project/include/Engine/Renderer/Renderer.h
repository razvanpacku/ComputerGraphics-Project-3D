#pragma once
#include <glad/glad.h>

#include "Engine/Resources/ResourceManager.h"
#include "RenderQueue.h"
#include "GLStateCache.h"
#include "ShadowFramebuffer.h"

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

	void RenderFrame();

	void ClearQueue() { renderQueue.Clear(); }
private:
	void DrawList(const std::vector<RenderSubmission>& submissions);
	void DrawSubmission(const RenderSubmission& submission);
	void DrawShadowSubmission(const RenderSubmission& submission);
	void DrawGUISubmission(const RenderSubmission& submission);
	
	App& app;
	ResourceManager& _rm;

	RenderQueue renderQueue;
	GLStateCache glState;

	ShadowFramebuffer
		pointShadowFBO	= ShadowFramebuffer(ShadowMapType::Point),
		dirShadowFBO	= ShadowFramebuffer(ShadowMapType::Directional);

	// Temporary functions
	void Clear() const;
	void Initialize();
	void RenderFunction();
	void Cleanup();
};

