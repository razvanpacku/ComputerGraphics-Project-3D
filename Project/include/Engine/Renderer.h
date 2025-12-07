#pragma once
#include <glad/glad.h>

#include "Engine/Resources/ResourceManager.h"

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
private:
	void Clear() const;

	App& app;
	ResourceManager& _rm;

	// Temporary functions
	void Initialize();
	void RenderFunction();
	void Cleanup();
};

