#pragma once
#define GLFW_INCLUDE_NONE
#include "Engine/InputEnums.h"

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

//forward declaration
class App;
class InputManagerAttorney;
class Window;

class InputManager
{
public:
    struct Connection {
        uint64_t id;
        std::function<void()> unbind;
        void Unbind() { if (unbind) unbind(); }
	};

    using Callback = std::function<void()>;
    using MouseMoveCallback = std::function<void(double x, double y)>;
    using MouseDeltaCallback = std::function<void(double dx, double dy)>;
    using MouseScrollCallback = std::function<void(double xoffset, double yoffset)>;

    static InputManager& Get();
	static void setApp(App* application);

    // ---- Key Bindings ----
    Connection BindKey(int key, InputEventType type, Callback cb);
    void UnbindKey(int key, InputEventType type, uint64_t id);

    // ---- Mouse Bindings ----
    Connection BindMouseButton(int button, InputEventType type, Callback cb);
    void UnbindMouseButton(int button, InputEventType type, uint64_t id);

    // ---- Mouse Move ----
    Connection BindMouseMove(MouseMoveCallback cb);
    Connection BindMouseDelta(MouseDeltaCallback cb);
    void UnbindMouseMove(uint64_t id);
    void UnbindMouseDelta(uint64_t id);

    // ---- Mouse Scroll ----
    Connection BindMouseScroll(MouseScrollCallback cb);
    void UnbindMouseScroll(uint64_t id);

	// ---- Mouse Modes ----
    void SetMouseMode(MouseMode mode);
	MouseMode GetMouseMode() const;
    void EnableRawMouse(bool enable);

    // ---- GLFW Callbacks ----
    void ProcessKey(int key, int action);
    void ProcessMouseButton(int button, int action);
    void ProcessMouseMove(double x, double y);
    void ProcessMouseScroll(double xoffset, double yoffset);

    // Update (used for Held detection)
    void Update();
private:
	InputManager() = default;

    uint64_t GenerateID();

    // Storage:
    std::unordered_map<int,
        std::unordered_map<InputEventType, std::vector<std::pair<uint64_t, Callback>>>> keyCallbacks;

    std::unordered_map<int,
        std::unordered_map<InputEventType, std::vector<std::pair<uint64_t, Callback>>>> mouseCallbacks;

    std::vector<std::pair<uint64_t, MouseMoveCallback>> mouseMoveCallbacks;

    std::vector<std::pair<uint64_t, MouseDeltaCallback>> mouseDeltaCallbacks;

    std::vector<std::pair<uint64_t, MouseScrollCallback>> mouseScrollCallbacks;

    // store last known mouse position
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool hasLastMousePos = false;
	bool ignoreNextDelta = false;

    // Key / mouse states for detecting Pressed/Held/Released
    std::unordered_map<int, bool> keyDown;
    std::unordered_map<int, bool> mouseDown;

    uint64_t nextID = 1;

	App* app = nullptr;

	friend class InputManagerAttorney;
};

typedef InputManager::Connection InputConnection;

class InputManagerAttorney {
private:
    static void SetIgnoreDelta(InputManager& im, bool ignore = true) {
        im.ignoreNextDelta = ignore;
    }
    friend class Window;
};
