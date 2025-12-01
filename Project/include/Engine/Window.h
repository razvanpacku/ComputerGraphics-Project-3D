#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

#include "InputEnums.h"

//forward declarations
class App;

class Window
{
public:
	Window(uint16_t width, uint16_t height, const std::string& name, int32_t argc, char** argv, App& app);
	~Window();

	void PollEvents();
	void SwapBuffers();
	bool ShouldClose() const;

	GLFWwindow* GetNative() const { return window; }
	MouseMode GetMouseMode() const;
	void SetMouseMode(MouseMode mode);

	void ToggleFullscreen();
private:
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

	bool isFullscreen = false;

	App& app;

	MouseMode mouseMode = MouseMode::Normal;

	uint16_t width, height;
	uint16_t windowPosX, windowPosY;
	GLFWwindow* window;
	GLFWmonitor* monitor;
	const GLFWvidmode* videoMode;
};

