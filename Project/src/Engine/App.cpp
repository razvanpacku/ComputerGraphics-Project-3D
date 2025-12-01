#include "Engine/App.h"

#include <chrono>

int32_t App::argc = 0;
char** App::argv = nullptr;

App::App(const std::string& name, uint16_t width, uint16_t height) : name(name)
{
	srand(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

	window = new Window(width, height, name, argc, argv, *this);
	renderer = new Renderer(*this);
}

App::~App()
{
	delete renderer;
	delete window;
}

void App::Init(int32_t argc, char** argv)
{
	App::argc = argc;
	App::argv = argv;
}

App& App::Get(const std::string& name, uint16_t width, uint16_t height)
{
	static App instance(name, width, height);
	return instance;
}

void App::Run()
{
	while(!window->ShouldClose())
	{
		if (glfwGetKey(window->GetNative(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window->GetNative(), true);

		renderer->Render();

		window->SwapBuffers();
		window->PollEvents();
	}
}
