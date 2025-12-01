#pragma once

enum class InputEventType {
    Pressed,
    Held,
    Released
};

enum class MouseMoveEvent {
    Move,       // absolute cursor movement
    Delta       // movement delta
};

enum class MouseScrollEvent {
    Scroll
};

enum class MouseMode {
    Normal,
    Hidden,
    Disabled,
};
