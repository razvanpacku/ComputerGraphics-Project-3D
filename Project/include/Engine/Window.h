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
private:
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

	App& app;

	uint16_t width, height;
	GLFWwindow* window;
};

