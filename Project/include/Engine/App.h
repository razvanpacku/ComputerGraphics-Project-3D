#pragma once
#include <cstdint>
#include <string>
#include <thread>

//forward declarations
class InputManager;
class Window;
class Renderer;
class BatchBuilder;
class Scene;

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

class App
{
public:
	static void Init(int32_t argc, char** argv);
	static App& Get(const std::string& name = "", uint16_t width = WINDOW_WIDTH, uint16_t height = WINDOW_HEIGHT);
	void SetScene(Scene* newScene);

	double DeltaTime() const { return deltaTime; }

	void RunLoop();
	void EventLoop();
	void Run();

	uint16_t GetWindowWidth() const;
	uint16_t GetWindowHeight() const;
	float GetWindowAspectRatio() const;
private:
	App(const std::string& name, uint16_t width, uint16_t height);
	~App();

	Window& GetWindow() { return *window; }
	Renderer& GetRenderer() { return *renderer; }

	static int32_t argc;
	static char** argv;

	std::string name;

	Window* window;
	Renderer* renderer;

	Scene* scene;
	bool firstFrame = true;

	double lastFrame = 0.;
	double deltaTime = 0.;

	friend class AppAttorney;
};

class AppAttorney {
	public:
	static Renderer& GetRenderer(App& app) {
		return app.GetRenderer();
	}
	static Window& GetWindow(App& app) {
		return app.GetWindow();
	}

	friend class Window;
	friend class Renderer;
	friend class InputManager;
	friend class BatchBuilder;
};

