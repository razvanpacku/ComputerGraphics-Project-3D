#pragma once

// ======================================================
// This header includes all component definitions used in the engine.
// It also defines concepts for internal and read-only components, and definitions for empty components.
// ======================================================

#include "NameComponent.h"
#include "HierarchyComponent.h"
#include "InternalNameComponent.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "RenderableComponent.h"

struct RootComponent {}; // An empty component to mark the root entity of a scene graph.

struct TransformDirtyTag {}; // An empty component to mark entities whose transform needs updating.
struct UITransformDirtyTag {}; // An empty component to mark UI entities whose transform needs updating.

struct TargetEntityTag {}; // An empty component to mark the entity used as a target by the camera.

// Internal components are components that certain entities must have and certain musn't have, so the user should not be able to add or remove them directly.
template<typename T>
concept InternalComponentType = std::is_same_v<T, InternalNameComponent> || std::is_same_v<T, HierarchyComponent> || std::is_same_v<T, RootComponent> || std::is_same_v<T, TransformDirtyTag>;

// Read-only components are components that the user can read but not modify directly.
template<typename T>
concept ReadOnlyComponentType = std::is_same_v<T, InternalNameComponent> || std::is_same_v<T, HierarchyComponent>;

// A helper struct to hold arguments for internal components.
template<InternalComponentType T, typename... Args>
struct InternalComp {
    using component_type = T;
    std::tuple<Args...> args;

    explicit InternalComp(Args&&... a)
        : args(std::forward<Args>(a)...)
    {
    }
};
