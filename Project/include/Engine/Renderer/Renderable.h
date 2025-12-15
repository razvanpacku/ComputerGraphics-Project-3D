#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <compare>

#include "Engine/Resources/ResourceManager.h"
#include "Engine/Components/Transform.h"
#include "Engine/Renderer//Culling/BoundingBox.h"

// ===================================================
// RenderLayer
//
// Enum defining the major render passes/layers.
// ===================================================
enum class RenderLayer : uint8_t
{
	Opaque,
	Transparent,
	GUI,
};

// ===================================================
// InstanceDataBase
//
// Struct holding per-instance data for instanced rendering.
// Served as a base for more specific instance data types.
// ===================================================
struct InstanceDataBase
{
	virtual ~InstanceDataBase() = default;
	uint32_t count = 0;
};

struct InstanceData : public InstanceDataBase
{
	std::vector<glm::mat4> modelMatrices;
};

struct GUIData {
	glm::vec4 uvOffset; // x, y, width, height in uv space
	glm::mat4 modelMatrix;
};

struct InstanceDataGUI : public InstanceDataBase
{
	std::vector<GUIData> guiData;
};

// ===================================================
// Renderable
//
// Lightweight draw item created by game code / systems.
// ==================================================
struct Renderable
{
	MeshManager::Handle meshHandle;
	MaterialManager::Handle materialHandle;

	Mesh* mesh = nullptr;           // optional direct mesh pointer for dynamic meshes that may be generated at runtime

	Transform transform;

	GLenum primitive = 0; // if 0, use mesh primitive

	// optional bounding box for culling
	BoundingBox aabb;
	bool hasBounds = false;

	bool cullBackfaces = true;

	bool castShadows = false;
	bool receiveShadows = false;

	InstanceDataBase* instanceData = nullptr; // optional instance data for instanced rendering

	RenderLayer layer = RenderLayer::Opaque;

	// sorting distance (filled at submission time)
	float sortDistance = 0.0f;

	// gui related data
	int16_t zOrder = 0;
	TextureManager::Handle textureHandle;
	glm::vec4 uvRect = glm::vec4(0.f, 0.f, 1.f, 1.f); // x, y, width, height in uv space
	glm::vec2 relativePosition = { 0.5f, 0.5f };
	glm::vec2 relativeSize = { 1.f, 1.f };
	glm::vec2 anchorPoint = { 0.5f, 0.5f };
	

	Renderable() = default;
	~Renderable();
	Renderable(const Renderable& other) noexcept;
	Renderable& operator=(const Renderable& other) noexcept = default;
	Renderable(Renderable&& other) noexcept;

	uint64_t GetSortKey() const;
};

// ===================================================
// RenderSubmission
//
// Internal representation including precomputed sort keys.
// Used by the renderer for sorting and rendering.
// ===================================================

struct RenderSubmission
{
	Renderable item;
	// precomputed sort key
	uint64_t sortKey = 0;
	// comparator for sorting
	std::strong_ordering operator<=>(const RenderSubmission& other) const noexcept;
};



