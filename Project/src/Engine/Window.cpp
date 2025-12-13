#include "Engine/Window.h"

#include "Engine/App.h"	
#include "Engine/Renderer/Renderer.h"
#include "Engine/InputManager.h"

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

	monitor = glfwGetPrimaryMonitor();
	videoMode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		throw std::runtime_error("GLFW window creation failed");
	}

	//put window in the center of the screen
	int screenW = videoMode->width;
	int screenH = videoMode->height;
	glfwSetWindowPos(window, (screenW - width) / 2, (screenH - height) / 2);
	windowPosX = static_cast<uint16_t>((screenW - width) / 2);
	windowPosY = static_cast<uint16_t>((screenH - height) / 2);

	ratio = static_cast<float>(width) / static_cast<float>(height);

	glfwMakeContextCurrent(window);

	glfwSetWindowUserPointer(window, &app);

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	// Disable V-Sync
	glfwSwapInterval(0);

	// Set initial mouse mode
	SetMouseMode(MouseMode::Disabled);
	InputManagerAttorney::SetIgnoreDelta(InputManager::Get()); // prevent large delta on first frame due to mouse mode

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

void Window::ToggleFullscreen() {
	isFullscreen = !isFullscreen;
	InputManagerAttorney::SetIgnoreDelta(InputManager::Get());
	
	if (isFullscreen) {

		// Save windowed position and size
		int _windowPosX, _windowPosY, _width, _height;
		glfwGetWindowPos(window, &_windowPosX, &_windowPosY);
		glfwGetWindowSize(window, &_width, &_height);
		windowPosX = static_cast<uint16_t>(_windowPosX);
		windowPosY = static_cast<uint16_t>(_windowPosY);
		width = static_cast<uint16_t>(_width);
		height = static_cast<uint16_t>(_height);

		ratio = static_cast<float>(videoMode->width) / static_cast<float>(videoMode->height);

		// Go fullscreen on primary monitor
		glfwSetWindowMonitor(window, monitor, 0, 0,
			videoMode->width, videoMode->height, videoMode->refreshRate);
		SetMouseMode(mouseMode);
	}
	else {
		// Restore windowed mode
		glfwSetWindowMonitor(window, nullptr,
			windowPosX, windowPosY, width, height, 0);
		SetMouseMode(mouseMode);

		ratio = static_cast<float>(width) / static_cast<float>(height);
	}

	// Update viewport and internal size
	int newW, newH;
	glfwGetFramebufferSize(window, &newW, &newH);
	glViewport(0, 0, newW, newH);

	// Reset cursor to center after toggle
	double centerX = newW / 2.0, centerY = newH / 2.0;

	// Reposition the OS cursor and reset last known mouse positions
	glfwSetCursorPos(window, centerX, centerY);
}

void Window::ResetViewport() {
	int fbW = 0, fbH = 0;
	glfwGetFramebufferSize(window, &fbW, &fbH);
	glViewport(0, 0, fbW, fbH);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0)
		return; // minimize case

	App& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
	Window& win = AppAttorney::GetWindow(app);

	glViewport(0, 0, width, height);

	if (!win.isFullscreen) {
		win.height = static_cast<uint16_t>(height);
		win.width = static_cast<uint16_t>(width);
		win.ratio = static_cast<float>(width) / static_cast<float>(height);
	}

	AppAttorney::GetRenderer(app).Render();
	win.SwapBuffers();
}

MouseMode Window::GetMouseMode() const {
	int mode = glfwGetInputMode(window, GLFW_CURSOR);
	if (mode == GLFW_CURSOR_HIDDEN)
		return MouseMode::Hidden;
	if (mode == GLFW_CURSOR_DISABLED)
		return MouseMode::Disabled;
	return MouseMode::Normal;
}

void Window::SetMouseMode(MouseMode mode) {
	int glfwMode = GLFW_CURSOR_NORMAL;
	if (mode == MouseMode::Hidden)   glfwMode = GLFW_CURSOR_HIDDEN;
	if (mode == MouseMode::Disabled) glfwMode = GLFW_CURSOR_DISABLED;
	glfwSetInputMode(window, GLFW_CURSOR, glfwMode);

	// Apply GLFW cursor mode first (may generate cursor events)
	glfwSetInputMode(window, GLFW_CURSOR, glfwMode);

	// Center the OS cursor in the framebuffer to avoid a huge first delta
	int fbW = 0, fbH = 0;
	glfwGetFramebufferSize(window, &fbW, &fbH);
	double centerX = (fbW > 0) ? fbW / 2.0 : width / 2.0;
	double centerY = (fbH > 0) ? fbH / 2.0 : height / 2.0;
	glfwSetCursorPos(window, centerX, centerY);

	// Ensure the InputManager ignores the next delta produced by this reposition
	InputManagerAttorney::SetIgnoreDelta(InputManager::Get());

	mouseMode = mode;
}