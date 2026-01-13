#pragma once
#include "ResourceManagerTemplate.h"
#include "MeshManager.h"
#include "MaterialManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Model : public IResource {
	struct MeshEntry {
		std::string name;            // Name from Assimp
		MeshManager::Handle mesh;    // Handle to the loaded mesh
		MaterialManager::Handle material; 
	};
	std::vector<MeshEntry> meshEntries;
	std::unordered_map<std::string, uint32_t> meshNameToIndex;
};

struct ModelResourceInfo {
	std::string modelFilePath;
};

class ModelPolicy : public IResourcePolicy<Model, ModelResourceInfo> {
public:
	using ResourceType = Model;
	using ResourceInfo = ModelResourceInfo;

	Model Create(const std::string& name, const ModelResourceInfo& resourceInfo) override;
	void Destroy(Model& res) override;

private:
	void LoadMaterialsFromJSON(const std::string& JSONFilePath, Model& model);
};

class ModelManager : public ResourceManagerTemplate<Model, ModelPolicy> {
public:
	using ModelHandle = Handle;

	virtual void PreloadResources(const std::string& resourceDirectory) override;
};

