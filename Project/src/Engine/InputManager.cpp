#include "Engine/InputManager.h"
#include <GLFW/glfw3.h>

#include "Engine/App.h"
#include "Engine/Window.h"

InputManager& InputManager::Get() {
    static InputManager instance;
    return instance;
}

void InputManager::setApp(App* application) {
	Get().app = application;
}

uint64_t InputManager::GenerateID() {
    return nextID++;
}

//
// ---------------------- Bindings ----------------------
//

InputConnection InputManager::BindKey(int key, InputEventType type, Callback cb) {
    uint64_t id = GenerateID();
    keyCallbacks[key][type].push_back({ id, cb });

    return { id, [=] { UnbindKey(key, type, id); } };
}

void InputManager::UnbindKey(int key, InputEventType type, uint64_t id) {
    auto& vec = keyCallbacks[key][type];
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [&](auto& x) { return x.first == id; }),
        vec.end());
}

// ---- Mouse ----

InputConnection InputManager::BindMouseButton(int button, InputEventType type, Callback cb) {
    uint64_t id = GenerateID();
    mouseCallbacks[button][type].push_back({ id, cb });

    return { id, [=] { UnbindMouseButton(button, type, id); } };
}

void InputManager::UnbindMouseButton(int button, InputEventType type, uint64_t id) {
    auto& vec = mouseCallbacks[button][type];
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [&](auto& x) { return x.first == id; }),
        vec.end());
}

// ---- Mouse Move ----

InputConnection InputManager::BindMouseMove(MouseMoveCallback cb) {
    uint64_t id = GenerateID();
    mouseMoveCallbacks.push_back({ id, cb });
    return { id, [=] { UnbindMouseMove(id); } };
}

void InputManager::UnbindMouseMove(uint64_t id) {
    mouseMoveCallbacks.erase(
        std::remove_if(mouseMoveCallbacks.begin(), mouseMoveCallbacks.end(),
            [&](auto& p) { return p.first == id; }),
        mouseMoveCallbacks.end());
}

// ---- Mouse Delta ----

InputConnection InputManager::BindMouseDelta(MouseDeltaCallback cb) {
    uint64_t id = GenerateID();
    mouseDeltaCallbacks.push_back({ id, cb });
    return { id, [=] { UnbindMouseDelta(id); } };
}

void InputManager::UnbindMouseDelta(uint64_t id) {
    mouseDeltaCallbacks.erase(
        std::remove_if(mouseDeltaCallbacks.begin(), mouseDeltaCallbacks.end(),
            [&](auto& p) { return p.first == id; }),
        mouseDeltaCallbacks.end());
}

// ---- Mouse Scroll ----

InputConnection InputManager::BindMouseScroll(MouseScrollCallback cb) {
    uint64_t id = GenerateID();
    mouseScrollCallbacks.push_back({ id, cb });
    return { id, [=] { UnbindMouseScroll(id); } };
}

void InputManager::UnbindMouseScroll(uint64_t id) {
    mouseScrollCallbacks.erase(
        std::remove_if(mouseScrollCallbacks.begin(), mouseScrollCallbacks.end(),
            [&](auto& p) { return p.first == id; }),
        mouseScrollCallbacks.end());
}

//
// ---------------------- Mouse Modes ----------------------
//

void InputManager::SetMouseMode(MouseMode mode) {
    auto& window = AppAttorney::GetWindow(*app);
	window.SetMouseMode(mode);
}

MouseMode InputManager::GetMouseMode() const {
    auto& window = AppAttorney::GetWindow(*app);
	return window.GetMouseMode();
}

void InputManager::EnableRawMouse(bool enable) {
    GLFWwindow* window = AppAttorney::GetWindow(*app).GetNative();
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, enable ? GLFW_TRUE : GLFW_FALSE);
}

//
// ---------------------- Input Processing ----------------------
//

void InputManager::ProcessKey(int key, int action) {
    bool isPressed = (action == GLFW_PRESS);
    bool isReleased = (action == GLFW_RELEASE);

    if (isPressed) {
        keyDown[key] = true;

        // Fire key pressed callbacks
        for (auto& [_, cb] : keyCallbacks[key][InputEventType::Pressed])
            cb();
    }
    else if (isReleased) {
        keyDown[key] = false;

        // Fire key released
        for (auto& [_, cb] : keyCallbacks[key][InputEventType::Released])
            cb();
    }
}

void InputManager::ProcessMouseButton(int button, int action) {
    bool isPressed = (action == GLFW_PRESS);
    bool isReleased = (action == GLFW_RELEASE);

    if (isPressed) {
        mouseDown[button] = true;

        for (auto& [_, cb] : mouseCallbacks[button][InputEventType::Pressed])
            cb();
    }
    else if (isReleased) {
        mouseDown[button] = false;

        for (auto& [_, cb] : mouseCallbacks[button][InputEventType::Released])
            cb();
    }
}

void InputManager::ProcessMouseMove(double x, double y) {
    // absolute movement callbacks
    for (auto& [_, cb] : mouseMoveCallbacks)
        cb(x, y);

    if (ignoreNextDelta || !hasLastMousePos) {
        // Reset last known position, ignore weird delta
		
        lastMouseX = x;
        lastMouseY = y;
        hasLastMousePos = true;
        ignoreNextDelta = false;

        return;
    }

    double dx = x - lastMouseX;
    double dy = y - lastMouseY;

    for (auto& [_, cb] : mouseDeltaCallbacks)
        cb(dx, dy);

    lastMouseX = x;
    lastMouseY = y;
}

void InputManager::ProcessMouseScroll(double xoffset, double yoffset) {
    for (auto& [_, cb] : mouseScrollCallbacks)
        cb(xoffset, yoffset);
}


//
// ---------------------- Update (Held Inputs) ----------------------
//

void InputManager::Update() {
    for (auto& [key, down] : keyDown) {
        if (down) {
            for (auto& [_, cb] : keyCallbacks[key][InputEventType::Held]) cb();
        }
    }

    for (auto& [button, down] : mouseDown) {
        if (down) {
            for (auto& [_, cb] : mouseCallbacks[button][InputEventType::Held]) cb();
        }
    }
}