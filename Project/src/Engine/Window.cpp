#include "Engine/Window.h"

#include "Engine/App.h"	

#include <iostream>

Window::Window(uint16_t width, uint16_t height, const std::string& name, int32_t argc, char** argv, App& app)
	: width(width), height(height), app(app)
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to init GLFW\n";
		throw std::runtime_error("GLFW initialization failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		throw std::runtime_error("GLFW window creation failed");
	}

	glfwMakeContextCurrent(window);

	glfwSetWindowUserPointer(window, &app);

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	// Load OpenGL functions with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		throw std::runtime_error("GLAD initialization failed");
	}
}

Window::~Window()
{
	if (window) {
		glfwDestroyWindow(window);
		glfwTerminate();
		window = nullptr;
	}
}

void Window::PollEvents() {
	glfwPollEvents();
}

void Window::SwapBuffers() {
	glfwSwapBuffers(window);
}

bool Window::ShouldClose() const {
	return glfwWindowShouldClose(window);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	App& app = *static_cast<App*>(glfwGetWindowUserPointer(window));

	glViewport(0, 0, width, height);
	AppAttorney::GetRenderer(app).Render();
	AppAttorney::GetWindow(app).SwapBuffers();
}