#include "Engine/Renderer/Renderable.h"
#include "Engine/Resources/ResourceManager.h"

// ==================================================
// Renderable
// ==================================================

Renderable::~Renderable()
{
	MeshPolicy mp;
	if(mesh)
	{
		mp.Destroy(*mesh);
		delete mesh;
	}
	if (instanceData)
	{
		delete instanceData;
	}
}

Renderable::Renderable(const Renderable& other) noexcept :
	meshHandle(other.meshHandle),
	materialHandle(other.materialHandle),
	mesh(other.mesh),
	transform(other.transform),
	primitive(other.primitive),
	aabb(other.aabb),
	hasBounds(other.hasBounds),
	castShadows(other.castShadows),
	receiveShadows(other.receiveShadows),
	cullBackfaces(other.cullBackfaces),
	instanceData(other.instanceData),
	layer(other.layer),
	zOrder(other.zOrder),
	textureHandle(other.textureHandle),
	uvRect(other.uvRect),
	relativePosition(other.relativePosition),
	relativeSize(other.relativeSize),
	anchorPoint(other.anchorPoint)
{
	// Note: shallow copy of mesh and instanceData pointers
}

Renderable::Renderable(Renderable&& other) noexcept :
	meshHandle(other.meshHandle),
	materialHandle(other.materialHandle),
	mesh(other.mesh),
	transform(std::move(other.transform)),
	primitive(other.primitive),
	aabb(other.aabb),
	hasBounds(other.hasBounds),
	castShadows(other.castShadows),
	receiveShadows(other.receiveShadows),
	cullBackfaces(other.cullBackfaces),
	instanceData(other.instanceData),
	layer(other.layer),
	zOrder(other.zOrder),
	textureHandle(other.textureHandle),
	uvRect(other.uvRect),
	relativePosition(other.relativePosition),
	relativeSize(other.relativeSize),
	anchorPoint(other.anchorPoint)
{
	other.mesh = nullptr;
	other.instanceData = nullptr;
}

uint64_t Renderable::GetSortKey() const
{
	auto& _rm = ResourceManager::Get();
	uint32_t materialId = materialHandle.id & 0xFFFF;
	auto shader = _rm.materials.Get(materialHandle)->GetShader();
	uint32_t shaderId = shader.id & 0xFFFF;

	uint32_t meshId = 0;
	if (mesh) {
		meshId = reinterpret_cast<uintptr_t>(mesh) & 0xFFFFFF;
	}
	else {
		meshId = meshHandle.id & 0xFFFFFF;
	}

	uint32_t flags = 0;
	flags |= cullBackfaces;

	uint64_t key = 0;
	if (layer != RenderLayer::GUI) {
		key |= (uint64_t(shaderId) << 48);
		key |= (uint64_t(materialId) << 32);
		key |= (uint64_t(meshId) << 8);
		key |= uint64_t(flags);
	}
	else {

		uint64_t zNormalized = static_cast<uint16_t>(zOrder) ^ 0x8000u;

		uint32_t textureId = textureHandle.id;

		key |= (zNormalized << 48);
		key |= (uint64_t(shaderId & 0xFFFF) << 32);
		key |= (uint64_t(materialId & 0xFFFF) << 16);
		key |= (uint64_t(textureId & 0x0FFF) << 4);
		key |= (uint64_t(flags & 0xF));
		// all GUI share the same mesh (a quad)
	}
	return key;
}

// ==================================================
// RenderSubmission
// ==================================================

std::strong_ordering RenderSubmission::operator<=>(const RenderSubmission& other) const noexcept
{
	return sortKey <=> other.sortKey;
}
