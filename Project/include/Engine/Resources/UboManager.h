#pragma once
#include "ResourceManagerTemplate.h"
#include "ShaderReflection.h"

#include "UboWriter.h"

// Forward declaration
class UboManager;

// Ubo was put into a separate file to avoid circular dependencies caused by UboWriter
#include "Ubo.h"

struct UboResourceInfo : public UniformBlockInfo {};

class UboPolicy : public IResourcePolicy<Ubo, UboResourceInfo> {
public:
	using ResourceType = Ubo;
	using ResourceInfo = UboResourceInfo;

	Ubo Create(const std::string& name, const UboResourceInfo& resourceInfo) override;
	void Destroy(Ubo& res) override;

private:
	static UboManager* _um;

	friend class UboManager;
};

class UboManager : public ResourceManagerTemplate<Ubo, UboPolicy> {
public:
	using UboHandle = Handle;

	UboManager();

	// We use a custom load function instead of the generic Load
	// because we want to automatically assign binding points
	// and also the name of the UBO will come from blockInfo.name and not from the user
	// UboWriters will also be created based on these UBOs
	UboHandle CreateOrGet(const UniformBlockInfo& blockInfo);

	UboWriter* GetUboWriter(const std::string& uboName);

	GLuint GetAndUseNextBindingPoint();
private:
	GLuint nextBindingPoint = 0;

	std::unordered_map<std::string, UboWriter> uboWriters;
};

