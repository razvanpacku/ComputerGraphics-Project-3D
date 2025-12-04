#include "Engine/Resources/MaterialManager.h"

#include "Engine/Resources/ResourceManager.h"

// =========================================================
// Material
// =========================================================
ShaderReflection* Material::GetShaderReflection() const {
	ShaderManager& sm = ResourceManager::Get().shaders;
	Shader* shaderPtr = sm.Get(shader);
	return shaderPtr ? &shaderPtr->reflection : nullptr;
}

void Material::SetTexture(const std::string& uniformName, TextureManager::Handle tex) {
	textures[uniformName] = tex;
}

void Material::setTexture(const std::string& uniformName, const std::string& texName) {
	TextureManager& tm = ResourceManager::Get().textures;
	TextureManager::Handle texHandle = tm.GetHandle(texName);
	textures[uniformName] = texHandle;
}

UboWriter* Material::GetLocalUboWriter(const std::string& blockName) {
	//check if UBO writer exists
	auto it = ubos.find(blockName);
	if (it != ubos.end()) {
		return &it->second;
	}
	return nullptr;
}

void Material::Apply() {
	ShaderManager& sm = ResourceManager::Get().shaders;
	TextureManager& tm = ResourceManager::Get().textures;

	Shader* shaderPtr = sm.Get(shader);
	if (!shaderPtr) return;
	sm.UseShader(*shaderPtr);

	// Apply uniforms
	for (const auto& [name, uniformValue] : uniforms) {
		//uniform name is guaranteed to exist in the shader from ManagerPolicy, so no check needed
		shaderPtr->SetRaw(name, uniformValue.type, uniformValue.data.data(), uniformValue.elementCount);
	}
	// Apply UBOs
	for (auto& [blockName, uboWriter] : ubos) {
		uboWriter.Upload();
	}
	// Bind textures
	for (const auto& [uniformName, texHandle] : textures) {
		//we can just bind all the textures, if the material has more textures than there are units, that's currently a limitation of the engine
		tm.Bind(texHandle);
	}
}

// =========================================================
// MaterialPolicy
// =========================================================
bool MaterialPolicy::IsSamplerType(GLenum t) {
	switch (t)
	{
	case GL_SAMPLER_2D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_3D:
	case GL_INT_SAMPLER_2D:
	case GL_UNSIGNED_INT_SAMPLER_2D:
		return true;
	default:
		return false;
	}
}

Material MaterialPolicy::Create(const std::string& name, const MaterialResourceInfo& resourceInfo) {
	Material mat;
	ShaderManager& sm = ResourceManager::Get().shaders;
	TextureManager& tm = ResourceManager::Get().textures;
	UboManager& um = ResourceManager::Get().ubos;

	auto& uboGlobalNames = UboManager::globalUboNames;

	// Get shader
	Shader* shaderPtr = sm.Get(resourceInfo.shaderName);
	if (!shaderPtr) {
		std::cerr << "MaterialPolicy::Create: Shader " << resourceInfo.shaderName << " not found for material " << name << std::endl;
		return mat;
	}

	mat.shader = sm.GetHandle(resourceInfo.shaderName);

	const ShaderReflection& reflection = shaderPtr->reflection;

	// Parse uniforms (+ samplers)
	for (const auto& [uniformName, uniformInfo] : reflection.uniforms) {
		if (IsSamplerType(uniformInfo.type)) {
			// This uniform is a SAMPLER
			mat.textures[uniformName] = {};  // invalid/sentinel handle
			continue;
		}

		// Allocate raw memory for the uniform
		UniformValue uvalue;
		uvalue.type = uniformInfo.type;
		uvalue.elementCount = static_cast<size_t>(uniformInfo.size);
		uvalue.data.resize(GLTypeSize(uniformInfo.type) * uvalue.elementCount, 0);

		mat.uniforms.try_emplace(uniformName, std::move(uvalue));
	}

	// Parse UBOs
	for (const auto& [blockName, blockInfo] : reflection.uniformBlocks) {
		// Skip global UBOs
		if (std::find(uboGlobalNames.begin(), uboGlobalNames.end(), blockName) != uboGlobalNames.end()) {
			continue;
		}
		// Construct key and value in-place to avoid any copy/move/assignment of UboWriter.
		mat.ubos.try_emplace(blockName, &blockInfo);
	}
	return mat;
}

void MaterialPolicy::Destroy(Material& res) {
	// Nothing special to do, all resources are managed elsewhere, and unordered_maps will clean themselves up
}
