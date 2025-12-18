#include "Engine/App.h"

#include "Engine/Window.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/InputManager.h"
#include "Engine/SceneGraph/Scene.h"

#include <chrono>
#include <iostream>

int32_t App::argc = 0;
char** App::argv = nullptr;

void KeyCallback(GLFWwindow*, int key, int sc, int action, int mods) {
	InputManager::Get().ProcessKey(key, action);
}

void MouseCallback(GLFWwindow*, int button, int action, int mods) {
	InputManager::Get().ProcessMouseButton(button, action);
}

void CursorPosCallback(GLFWwindow*, double x, double y) {
	InputManager::Get().ProcessMouseMove(x, y);
}

void ScrollCallback(GLFWwindow*, double xoff, double yoff) {
	InputManager::Get().ProcessMouseScroll(xoff, yoff);
}

App::App(const std::string& name, uint16_t width, uint16_t height) : name(name)
{
	srand(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

	//init InputManager with this app
	InputManager::setApp(this);

	window = new Window(width, height, name, argc, argv, *this);

	//load resources
	ResourceManager::Get().PreloadResources("resources/");

	renderer = new Renderer(*this);

	glfwSetKeyCallback(window->GetNative(), KeyCallback);
	glfwSetMouseButtonCallback(window->GetNative(), MouseCallback);
	glfwSetCursorPosCallback(window->GetNative(), CursorPosCallback);
	glfwSetScrollCallback(window->GetNative(), ScrollCallback);

	InputManager::Get().BindKey(GLFW_KEY_ESCAPE, InputEventType::Pressed, [this]() {
		this->window->PollEvents();
		glfwSetWindowShouldClose(this->window->GetNative(), true);
		});
	InputManager::Get().BindKey(GLFW_KEY_F11, InputEventType::Pressed, [this]() {
		this->window->ToggleFullscreen();
		});
}

App::~App()
{
	delete renderer;
	delete window;
	if (scene) delete scene;
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
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		InputManager::Get().Update();

		if(firstFrame && scene)
		{
			deltaTime = 0.0;
			firstFrame = false;
		}
		else if(!firstFrame && scene)
		{
			scene->Update(deltaTime);
		}

		renderer->Render();

		window->SwapBuffers();
		window->PollEvents();
	}
}

void App::SetScene(Scene* newScene)
{
	if (scene)
	{
		delete scene;
	}
	scene = newScene;
	if (scene)
	{
		scene->Init({renderer});
		Scene::SetActiveScene(scene);
		firstFrame = true;
	}
	else {
		Scene::SetActiveScene(nullptr);
	}
}
uint16_t App::GetWindowWidth() const
{
	return window->GetWidth();
}

uint16_t App::GetWindowHeight() const
{
	return window->GetHeight();
}

float App::GetWindowAspectRatio() const
{
	return window->GetAspectRatio();
}