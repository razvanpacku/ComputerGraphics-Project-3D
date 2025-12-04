#include "Engine/Resources/UboManager.h"

// ==========================================================
// UboPolicy
// ==========================================================
UboManager* UboPolicy::_um = nullptr;

Ubo UboPolicy::Create(const std::string& name, const UboResourceInfo& resourceInfo)
{
	Ubo ubo;

	// resourceInfo is obtained from UboManager::CreateOrGet, which modifies the UniformBlockInfo obtained from shader reflection by assigning a binding point
	ubo.name = resourceInfo.name;
	ubo.index = resourceInfo.index;
	ubo.binding = resourceInfo.binding;
	ubo.dataSize = resourceInfo.dataSize;
	ubo.fields = resourceInfo.fields;

	ubo.alive = true;

	//bind the UBO to the assigned binding point
	glGenBuffers(1, &ubo.bufferID);
	if(!ubo.bufferID)
	{
		std::cerr << "Failed to generate UBO buffer for " << name << std::endl;
		return ubo;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, ubo.bufferID);
	glBufferData(GL_UNIFORM_BUFFER, ubo.dataSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, ubo.binding, ubo.bufferID);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return ubo;
}

void UboPolicy::Destroy(Ubo& res)
{
	if (res.bufferID != 0) {
		glDeleteBuffers(1, &res.bufferID);
		res.bufferID = 0;
	}
}

// ==========================================================
// UboManager
// ==========================================================
const std::vector<std::string> UboManager::globalUboNames = {
	"Matrices",
	"Lighting",
	"Time"
};

UboManager::UboManager()
{
	UboPolicy::_um = this;
}

UboManager::UboHandle UboManager::CreateOrGet(const UniformBlockInfo& blockInfo)
{
	// Check if UBO with this name already exists
	if (Exists(blockInfo.name))
		return GetHandle(blockInfo.name);
	// Create UboResourceInfo
	UboResourceInfo resourceInfo;
	resourceInfo.name = blockInfo.name;
	resourceInfo.index = blockInfo.index;
	resourceInfo.dataSize = blockInfo.dataSize;
	resourceInfo.fields = blockInfo.fields;
	resourceInfo.blockSize = blockInfo.blockSize;
	// Assign next binding point
	resourceInfo.binding = GetAndUseNextBindingPoint();
	// Load the UBO using the ResourceManagerTemplate's Load function
	UboManager::UboHandle uboHandle = Load(blockInfo.name, resourceInfo);

	// Create corresponding UboWriter
	uboWriters.emplace(blockInfo.name, UboWriter(Get(uboHandle)));

	return uboHandle;
}

UboWriter* UboManager::GetUboWriter(const std::string& uboName)
{
	auto it = uboWriters.find(uboName);
	if (it == uboWriters.end())
		return nullptr;
	return &it->second;
}

GLuint UboManager::GetAndUseNextBindingPoint()
{
	return nextBindingPoint++;
}