#pragma once
#include "ResourceManagerTemplate.h"
#include "ShaderReflection.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboWriter.h"

#include <json.hpp>

#include <span>
#include <vector>

//forward declaration
class MaterialPolicy;
class MaterialManager;
class ModelPolicy;
struct GLStateCache;

struct Material : public IResource {
public:
	Material() = default;
	~Material() override = default;

	ShaderManager::Handle GetShader() const { return shader; }
	ShaderReflection* GetShaderReflection() const;

	template<typename T>
	void SetUniform(const std::string& name, const T& value, size_t index = 0) {
		uniforms[name].Set(value, index);
	}

	template<typename T>
	void SetUniformArray(const std::string& name, const T* arr) {
		uniforms[name].SetArray(arr);
	}

	template<typename T>
	void SetUniformArray(const std::string& name, std::span<const T> values) {
		uniforms[name].SetArray(values.data());
	}

	template<typename T>
	void SetUniformArray(const std::string& name, std::initializer_list<T> init) {
		SetUniformArray(name, std::span<const T>(init.begin(), init.size()));
	}

	template<typename T>
	void SetUniformArray(const std::string& name, const std::vector<T>& values) {
		uniforms[name].SetArray(values.data());
	}

	void SetTexture(const std::string& uniformName, TextureManager::Handle tex);
	void setTexture(const std::string& uniformName, const std::string& texName);
	UboWriter* GetLocalUboWriter(const std::string& blockName);

	void Apply(GLStateCache* glState = nullptr);


private:
	ShaderManager::Handle shader;

	std::unordered_map<std::string, UniformValue> uniforms;				// local uniform values for storing non-UBO uniform data local to the material
	std::unordered_map<std::string, UboWriter> ubos;					// local UBO writers for storing UBO data local to the material
	std::unordered_map<std::string, TextureManager::Handle> textures;	// texture samplers

	std::unordered_map<std::string, bool> dirtyTextures;				// track which textures have been modified

	friend class MaterialPolicy;
};

struct MaterialResourceInfo {
	std::string shaderName;						// name of the shader to use
};

class MaterialPolicy : public IResourcePolicy<Material, MaterialResourceInfo> {
public:
	using ResourceInfo = MaterialResourceInfo;
	using ResourceType = Material;

	Material Create(const std::string& name, const MaterialResourceInfo& resourceInfo) override;
	void Destroy(Material& res) override;

	// returns Material, material name and error string (empty if no error)
	std::tuple<Material, std::string, std::string> CreateFromJSON(const std::string& JSONFilePath);

	std::tuple<Material, std::string, std::string> CreateFromJSONObject(const nlohmann::json& j);

	void ParseJSON(const nlohmann::json& j, Material& mat);
};

class MaterialManager : public ResourceManagerTemplate<Material, MaterialPolicy> {
public:
	using MaterialHandle = Handle;

	MaterialHandle LoadFromJSON(const std::string& JSONFilePath);
	virtual void PreloadResources(const std::string& resourceDirectory) override;

	friend class ModelPolicy;
};

