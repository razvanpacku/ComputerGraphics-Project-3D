#pragma once
#include "ResourceManagerTemplate.h"
#include "ShaderReflection.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UboWriter.h"

#include <span>

//forward declaration
class MaterialPolicy;
class MaterialManager;

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

	void SetTexture(const std::string& uniformName, TextureManager::Handle tex);
	void setTexture(const std::string& uniformName, const std::string& texName);
	UboWriter* GetLocalUboWriter(const std::string& blockName);

	void Apply();


private:
	ShaderManager::Handle shader;

	std::unordered_map<std::string, UniformValue> uniforms;				// local uniform values for storing non-UBO uniform data local to the material
	std::unordered_map<std::string, UboWriter> ubos;					// local UBO writers for storing UBO data local to the material
	std::unordered_map<std::string, TextureManager::Handle> textures;

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
private:
	bool IsSamplerType(GLenum t);
};

class MaterialManager : public ResourceManagerTemplate<Material, MaterialPolicy> {};

