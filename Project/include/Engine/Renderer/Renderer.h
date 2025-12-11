#pragma once
#include <glad/glad.h>

#include "Engine/Resources/ResourceManager.h"
#include "RenderQueue.h"
#include "GLStateCache.h"

#define DEFAULT_CLEAR_COLOR_R 0.0f
#define DEFAULT_CLEAR_COLOR_G 0.0f
#define DEFAULT_CLEAR_COLOR_B 0.0f

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
	
	App& app;
	ResourceManager& _rm;

	RenderQueue renderQueue;
	GLStateCache glState;

	// Temporary functions
	void Clear() const;
	void Initialize();
	void RenderFunction();
	void Cleanup();
};

