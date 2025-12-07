#include "Engine/Resources/ModelManager.h"
#include "Engine/Resources/ResourceManager.h"

#include <glm/glm.hpp>
#include <filesystem>
#include <fstream>

// =========================================================
// ModelPolicy
// =========================================================
Model ModelPolicy::Create(const std::string& name, const ModelResourceInfo& resourceInfo)
{
	Model model;

	auto& _rm = ResourceManager::Get();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(resourceInfo.modelFilePath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices);

	bool flipYandZ = false;

	//glTF files use a different coordinate system, so we flip Y and Z axes
	// get it from file extension
	std::filesystem::path filePath(resourceInfo.modelFilePath);
	if (filePath.extension() == ".gltf" || filePath.extension() == ".glb") {
		flipYandZ = true;
	}

	if (!scene || !scene->HasMeshes()) {
		std::cerr << "Failed to load model: " << resourceInfo.modelFilePath << "\n";
		return model;
	}

	model.meshEntries.reserve(scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* ai_mesh = scene->mMeshes[i];

		struct Vertex {
			glm::vec4 position;
			glm::vec2 uv;
			glm::vec3 normal;
		};

		std::vector<VertexAttribute> attributes = {
			{ 0, 4, GL_FLOAT, offsetof(Vertex, position), GL_FALSE },
			{ 1, 2, GL_FLOAT, offsetof(Vertex, uv), GL_FALSE },
			{ 2, 3, GL_FLOAT, offsetof(Vertex, normal), GL_FALSE },
		};

		std::vector<Vertex> vertices;
		for (unsigned int v = 0; v < ai_mesh->mNumVertices; ++v) {
			Vertex vert{};
			if (flipYandZ)
				vert.position = glm::vec4(ai_mesh->mVertices[v].x, -ai_mesh->mVertices[v].z, ai_mesh->mVertices[v].y, 1.0f);
			else
				vert.position = glm::vec4(ai_mesh->mVertices[v].x, ai_mesh->mVertices[v].y, ai_mesh->mVertices[v].z, 1.0f);
			if (ai_mesh->HasNormals()) {
				if (flipYandZ)
					vert.normal = glm::vec3(ai_mesh->mNormals[v].x, -ai_mesh->mNormals[v].z, ai_mesh->mNormals[v].y);
				else
					vert.normal = glm::vec3(ai_mesh->mNormals[v].x, ai_mesh->mNormals[v].y, ai_mesh->mNormals[v].z);
			}
			if (ai_mesh->HasTextureCoords(0))
				vert.uv = glm::vec2(ai_mesh->mTextureCoords[0][v].x, ai_mesh->mTextureCoords[0][v].y);
			vertices.push_back(vert);
		}

		// Convert indices
		std::vector<uint32_t> indices;
		for (unsigned int f = 0; f < ai_mesh->mNumFaces; ++f) {
			aiFace face = ai_mesh->mFaces[f];
			indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
		}

		// Flatten vertex buffer
		std::vector<uint8_t> vertexData(vertices.size() * sizeof(Vertex));
		memcpy(vertexData.data(), vertices.data(), vertexData.size());

		std::string meshName;
		if (ai_mesh->mName.length > 0) {
			meshName = ai_mesh->mName.C_Str();
		}
		else {
			meshName = "mesh_" + std::to_string(i);
		}
		std::string managerName = name + "_" + meshName;
		MeshManager::Handle meshHandle = _rm.meshes.Load(
			managerName,
			MeshResoruceInfo{
				vertexData,
				indices,
				attributes,
				sizeof(Vertex)
			});

		Model::MeshEntry meshEntry;
		meshEntry.name = meshName;
		meshEntry.mesh = meshHandle;
		model.meshEntries.push_back(meshEntry);
		model.meshNameToIndex[meshName] = static_cast<uint32_t>(i);
	}

	// try to load materials from JSON file
	std::filesystem::path modelPath(resourceInfo.modelFilePath);
	std::filesystem::path parentDir = modelPath.parent_path();
	std::string materialFilePath2 = (parentDir / ("mat.json")).string();

	if (std::filesystem::exists(materialFilePath2)) {
		LoadMaterialsFromJSON(materialFilePath2, model);
	}

	model.alive = true;
	return model;
}

void ModelPolicy::Destroy(Model& res)
{
	auto& _rm = ResourceManager::Get();
	for (auto& meshEntry : res.meshEntries) {
		_rm.meshes.Remove(meshEntry.mesh);
	}
	res.meshEntries.clear();
	res.alive = false;
}

void ModelPolicy::LoadMaterialsFromJSON(const std::string& JSONFilePath, Model& model)
{
	MaterialManager& mm = ResourceManager::Get().materials;
	MaterialPolicy& mp = mm.policy;
	using json = nlohmann::json;

	std::ifstream file(JSONFilePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open material JSON: " << JSONFilePath << "\n";
		return;
	}

	json j;
	try {
		file >> j;
	}
	catch (const std::exception&) {
		std::cerr << "Failed to parse material JSON: " << JSONFilePath << "\n";
		return;
	}

	std::unordered_map<std::string, MaterialManager::Handle> loadedMaterials;

	if(j.contains("materials") && j["materials"].is_array()) {
		for (const auto& matEntry : j["materials"]) {
			auto [mat, matName, err] = mp.CreateFromJSONObject(matEntry);

			if (!err.empty()) {
				if (err != "MaterialExists") {
					std::cerr << "ModelPolicy::LoadMaterialsFromJSON: Failed to create material from JSON entry: " << err << "\n";
					continue;
				}
				else {
					mat = *mm.Get(matName);
				}
			}

			loadedMaterials[matName] = mm.Register(matName, mat);
		}
	}

	// Assign materials to model mesh entries
	if (j.contains("meshMaterialMap")) {
		for (auto& [meshName, materialName] : j["meshMaterialMap"].items()) {

			if (!loadedMaterials.contains(materialName))
				continue;

			if (!model.meshNameToIndex.contains(meshName))
				continue;

			uint32_t idx = model.meshNameToIndex[meshName];
			model.meshEntries[idx].material = loadedMaterials[materialName];
		}
	}
}

void ModelManager::PreloadResources(const std::string& resourceDirectory)
{
	std::string shaderDir = "models/";

	std::filesystem::path fullDir = std::filesystem::path(resourceDirectory) / shaderDir;

	// models are stored as directories which inside have the .glb and mat.json files
	// model will be named after the directory
	for (const auto& entry : std::filesystem::directory_iterator(fullDir)) {
		if (entry.is_directory()) {
			std::string modelName = entry.path().filename().string();
			std::string modelPath = (entry.path() / (modelName + ".glb")).string();
			std::cout << "Loading model: " << modelName << " ("
				<< modelPath << ")\n";
			ModelResourceInfo modelInfo;
			modelInfo.modelFilePath = modelPath;
			Load(modelName, modelInfo);
		}
	}
}