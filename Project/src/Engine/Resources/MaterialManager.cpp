#include "Engine/Resources/MaterialManager.h"

#include "Engine/Resources/ResourceManager.h"
#include "Engine/Renderer/GLStateCache.h"

#include <filesystem>
#include <fstream>

// Macro to parse an array JSON into a given glm type.
// Usage: PARSE_GLM_SINGLE(glm::vec2)
#define PARSE_GLM_SINGLE(GLM_TYPE) \
do { \
	GLM_TYPE dat = glm_from_floats<GLM_TYPE>(floatValues); \
	mat.SetUniform(name, dat); \
} while(0)

//Dito for UboWriters
#define PARSE_GLM_SINGLE_WRITER(GLM_TYPE) \
do { \
	GLM_TYPE dat = glm_from_floats<GLM_TYPE>(floatValues); \
	writer->Set(fieldName, dat); \
} while(0)

// Macro to parse an array-of-arrays JSON into a std::vector of a given glm type.
// Usage: PARSE_GLM_ARRAY(glm::vec2)
// It expects `val` to be the nlohmann::json array being iterated and `name`/`mat` in scope.
#define PARSE_GLM_ARRAY(GLM_TYPE) \
do { \
    std::vector<GLM_TYPE> vecs; \
    for (const auto& elem : val) { \
        std::vector<float> floatValues; \
        for (const auto& f : elem) { \
            floatValues.push_back(f.get<float>()); \
        } \
        GLM_TYPE dat = glm_from_floats<GLM_TYPE>(floatValues); \
        vecs.push_back(dat); \
    } \
    mat.SetUniformArray(name, vecs); \
} while(0)

// Dito for UboWriters
#define PARSE_GLM_ARRAY_WRITER(GLM_TYPE) \
do { \
    std::vector<GLM_TYPE> vecs; \
    for (const auto& elem : val) { \
        std::vector<float> floatValues; \
        for (const auto& f : elem) { \
            floatValues.push_back(f.get<float>()); \
        } \
        GLM_TYPE dat = glm_from_floats<GLM_TYPE>(floatValues); \
        vecs.push_back(dat); \
    } \
    writer->SetArray(fieldName, vecs); \
} while(0)

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
	if(texHandle == textures[uniformName]) {
		//same texture, do nothing
		return;
	}
	textures[uniformName] = texHandle;
	dirtyTextures[uniformName] = true;
}

UboWriter* Material::GetLocalUboWriter(const std::string& blockName) {
	//check if UBO writer exists
	auto it = ubos.find(blockName);
	if (it != ubos.end()) {
		return &it->second;
	}
	return nullptr;
}

void Material::Apply(GLStateCache* glState) {
	ShaderManager& sm = ResourceManager::Get().shaders;
	TextureManager& tm = ResourceManager::Get().textures;
	MaterialManager& mm = ResourceManager::Get().materials;

	Shader* shaderPtr = nullptr;
	shaderPtr = sm.Get(shader);
	if (!shaderPtr) return;
	if (!glState || glState->currentShader != shader) {
		sm.UseShader(*shaderPtr);
		glState->currentShader = shader;
	}

	// Apply uniforms
	for (const auto& [name, uniformValue] : uniforms) {
		//uniform name is guaranteed to exist in the shader from ManagerPolicy, so no check needed
		if (!uniformValue.dirty && (!glState || glState->currentMaterial == this)) continue; //skip non-dirty uniforms
		shaderPtr->SetRaw(name, uniformValue.type, uniformValue.data.data(), uniformValue.elementCount);
		uniforms[name].dirty = false;
	}
	// Apply UBOs
	for (auto& [blockName, uboWriter] : ubos) {
		if (!glState || glState->currentMaterial != this) uboWriter.MakeDirty();
		uboWriter.Upload();
	}
	// Bind textures
	for (const auto& [uniformName, texHandle] : textures) {
		//only bind if dirty
		if (!dirtyTextures[uniformName] && glState->currentMaterial == this) continue;

		//we can just bind all the textures, if the material has more textures than there are units, that's currently a limitation of the engine
		//tm.Bind(texHandle);
		const ShaderReflection& refl = shaderPtr->reflection;
		auto it = refl.samplers.find(uniformName);
		if (it == refl.samplers.end()) continue;

		int unit = it->second.textureUnit;

		bool bindUnit = (!glState) || (glState->activeTextureUnit != unit);

		if(glState && glState->boundTextures[unit] == texHandle) {
			//texture already bound at this unit, skip
			dirtyTextures[uniformName] = false;
			continue;
		}

		//if texture handle is invalid, unbind the texture at the unit
		if (!texHandle.IsValid()) {
			tm.UnbindFromUnit(unit, bindUnit);
			if(glState) glState->boundTextures[unit] = TextureManager::Handle{};
			dirtyTextures[uniformName] = false;
			continue;
		}

		tm.Bind(texHandle, unit, bindUnit);
		//debug check what texture is bound at this unit
		if (glState) {
			glState->boundTextures[unit] = texHandle;
		}
		dirtyTextures[uniformName] = false;

	}

	if (glState) glState->currentMaterial = this;
}

// =========================================================
// MaterialPolicy
// =========================================================
Material MaterialPolicy::Create(const std::string& name, const MaterialResourceInfo& resourceInfo) {
	Material mat;
	ShaderManager& sm = ResourceManager::Get().shaders;
	TextureManager& tm = ResourceManager::Get().textures;
	UboManager& um = ResourceManager::Get().ubos;

	auto& uboGlobalNames = UboManager::globalUboNames;

	std::string modelDirectory = std::filesystem::path(resourceInfo.shaderName).parent_path().string();

	// Get shader
	Shader* shaderPtr = sm.Get(resourceInfo.shaderName);
	if (!shaderPtr) {
		std::cerr << "MaterialPolicy::Create: Shader " << resourceInfo.shaderName << " not found for material " << name << std::endl;
		return mat;
	}

	mat.shader = sm.GetHandle(resourceInfo.shaderName);

	const ShaderReflection& reflection = shaderPtr->reflection;

	// Parse uniforms
	for (const auto& [uniformName, uniformInfo] : reflection.uniforms) {

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

	// Parse samplers
	for (const auto& [samplerName, samplerInfo] : reflection.samplers) {
		// Just set to invalid handle for now, user can set later
		mat.textures[samplerName] = TextureManager::Handle{};
		mat.dirtyTextures[samplerName] = false;
	}
	return mat;
}

void MaterialPolicy::Destroy(Material& res) {
	// Nothing special to do, all resources are managed elsewhere, and unordered_maps will clean themselves up
	res.alive = false;
}

std::tuple<Material, std::string, std::string> MaterialPolicy::CreateFromJSON(const std::string& JSONFilePath) {
	Material mat;
	std::string managerName;
	std::string error;

	ShaderManager& sm = ResourceManager::Get().shaders;
	using json = nlohmann::json;

	std::ifstream file(JSONFilePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open material JSON: " << JSONFilePath << "\n";
		error = "FileMissing";
		return std::make_tuple(mat, managerName, "error");
	}

	json j;
	try {
		file >> j;
	}
	catch (const std::exception&) {
		std::cerr << "Failed to parse material JSON: " << JSONFilePath << "\n";
		error = "FileInvalid";
		return std::make_tuple(mat, managerName, error);
	}

	//load name
	if(!j.contains("name") || !j["name"].is_string()) {
		error = "NameMissing";
		return std::make_tuple(mat, managerName, error);
	}
	managerName = j["name"].get<std::string>();

	//check if material with this name already exists
	if (ResourceManager::Get().materials.Exists(managerName)) {
		error = "MaterialExists";
		return std::make_tuple(mat, managerName, error);
	}

	//load shader
	if (!j.contains("shader") || !j["shader"].is_string()) {
		error = "ShaderMissing";
		return std::make_tuple(mat, managerName, error);
	}
	std::string shaderName = j["shader"].get<std::string>();
	ShaderManager::Handle shaderHandle = sm.GetHandle(shaderName);
	if (!shaderHandle.IsValid()) {
		error = "ShaderNotFound";
		return std::make_tuple(mat, managerName, error);
	}

	// create material from shader, which will be later populated with data from JSON
	mat = Create(managerName, MaterialResourceInfo{ shaderName });

	ParseJSON(j, mat);

	return std::make_tuple(mat, managerName, error);
}

std::tuple<Material, std::string, std::string> MaterialPolicy::CreateFromJSONObject(const nlohmann::json& j) {
	Material mat;
	std::string managerName;
	std::string error;

	ShaderManager& sm = ResourceManager::Get().shaders;
	using json = nlohmann::json;

	//load name
	if (!j.contains("name") || !j["name"].is_string()) {
		error = "NameMissing";
		return std::make_tuple(mat, managerName, error);
	}
	managerName = j["name"].get<std::string>();

	//check if material with this name already exists
	if (ResourceManager::Get().materials.Exists(managerName)) {
		error = "MaterialExists";
		return std::make_tuple(mat, managerName, error);
	}

	//load shader
	if (!j.contains("shader") || !j["shader"].is_string()) {
		error = "ShaderMissing";
		return std::make_tuple(mat, managerName, error);
	}
	std::string shaderName = j["shader"].get<std::string>();
	ShaderManager::Handle shaderHandle = sm.GetHandle(shaderName);
	if (!shaderHandle.IsValid()) {
		error = "ShaderNotFound";
		return std::make_tuple(mat, managerName, error);
	}

	// create material from shader, which will be later populated with data from JSON
	mat = Create(managerName, MaterialResourceInfo{ shaderName });

	ParseJSON(j, mat);

	return std::make_tuple(mat, managerName, error);
}

void MaterialPolicy::ParseJSON(const nlohmann::json& j, Material& mat) {
	ShaderManager& sm = ResourceManager::Get().shaders;
	using json = nlohmann::json;

	ShaderReflection& refl = sm.Get(mat.shader)->reflection;

	//load uniforms
	if (j.contains("uniforms") && j["uniforms"].is_object()) {
		for (auto& [name, val] : j["uniforms"].items())
		{
			if (!refl.uniforms.contains(name)) {
				std::cerr << "[Material JSON] Shader has no uniform: " << name << "\n";
				continue;
			}

			// Scalar float / int / bool
			if (val.is_number_float()) {
				mat.SetUniform(name, val.get<float>());
			}
			else if (val.is_number_integer()) {
				mat.SetUniform(name, val.get<int>());
			}
			// Array or vector/matrix
			else if (val.is_array())
			{
				// check if uniform is array
				const UniformInfo* uinfo = refl.GetUniform(name);
				if (uinfo->size == 1) {
					// not an array, must be vector/matrix
					// check that size matches expected size from uniform type
					size_t expectedSize = GLTypeSize(uinfo->type) / sizeof(float);
					if (val.size() != expectedSize) {
						std::cerr << "[Material JSON] Uniform " << name << " size mismatch, expected size " << expectedSize << ".\n";
						continue;
					}

					// types will be treated as floats since that's what we use as glm types
					std::vector<float> floatValues;
					for (const auto& elem : val) {
						floatValues.push_back(elem.get<float>());
					}
					// convert to appropriate glm type based on uniform type
					switch (uinfo->type) {
					case GL_FLOAT_VEC2:
					case GL_INT_VEC2:
					case GL_BOOL_VEC2:
						PARSE_GLM_SINGLE(glm::vec2);
						break;
					case GL_FLOAT_VEC3:
					case GL_INT_VEC3:
					case GL_BOOL_VEC3:
						PARSE_GLM_SINGLE(glm::vec3);
						break;
					case GL_FLOAT_VEC4:
					case GL_INT_VEC4:
					case GL_BOOL_VEC4:
						PARSE_GLM_SINGLE(glm::vec4);
						break;
					case GL_FLOAT_MAT2:
						PARSE_GLM_SINGLE(glm::mat2);
						break;
					case GL_FLOAT_MAT3:
						PARSE_GLM_SINGLE(glm::mat3);
						break;
					case GL_FLOAT_MAT4:
						PARSE_GLM_SINGLE(glm::mat4);
						break;
					case GL_FLOAT_MAT2x3:
						PARSE_GLM_SINGLE(glm::mat2x3);
						break;
					case GL_FLOAT_MAT3x2:
						PARSE_GLM_SINGLE(glm::mat3x2);
						break;
					case GL_FLOAT_MAT2x4:
						PARSE_GLM_SINGLE(glm::mat2x4);
						break;
					case GL_FLOAT_MAT4x2:
						PARSE_GLM_SINGLE(glm::mat4x2);
						break;
					case GL_FLOAT_MAT3x4:
						PARSE_GLM_SINGLE(glm::mat3x4);
						break;
					case GL_FLOAT_MAT4x3:
						PARSE_GLM_SINGLE(glm::mat4x3);
						break;
					default:
						std::cerr << "[Material JSON] Uniform " << name << " has unsupported type for vector/matrix.\n";
						break;

					}
				}
				else if (uinfo->size > 1) {
					// check if json array has the right number of elements
					if (val.size() != static_cast<size_t>(uinfo->size)) {
						std::cerr << "[Material JSON] Uniform array " << name << " size mismatch, expected size " << uinfo->size << ".\n";
						continue;
					}

					// check if all elements are of the same type: int/bool, float or array(vector/matrix)
					bool allInt = true;
					bool allFloat = true;
					bool allArray = true;
					for (const auto& elem : val) {
						if (!elem.is_number_integer()) allInt = false;
						if (!elem.is_number_float()) allFloat = false;
						if (!elem.is_array()) allArray = false;
					}

					if (!allInt && !allFloat && !allArray) {
						std::cerr << "[Material JSON] Uniform array " << name << " has mixed element types.\n";
						continue;
					}

					if (allInt) {
						if (!IsIntegerType(uinfo->type)) {
							std::cerr << "[Material JSON] Uniform array " << name << " type mismatch, expected integer type.\n";
							continue;
						}

						std::vector<int> intValues;
						for (const auto& elem : val) {
							intValues.push_back(elem.get<int>());
						}
						mat.SetUniformArray(name, intValues);
					}
					else if (allFloat) {
						if (!IsFloatType(uinfo->type)) {
							std::cerr << "[Material JSON] Uniform array " << name << " type mismatch, expected float type.\n";
							continue;
						}

						std::vector<float> floatValues;
						for (const auto& elem : val) {
							floatValues.push_back(elem.get<float>());
						}
						mat.SetUniformArray(name, floatValues);
					}
					else if (allArray) {
						// first check that all of them have the same size
						size_t arraySize = val[0].size();
						bool sizeMismatch = false;
						for (const auto& elem : val) {
							if (elem.size() != arraySize) {
								sizeMismatch = true;
								break;
							}
						}

						if (sizeMismatch) {
							std::cerr << "[Material JSON] Uniform array " << name << " has elements with mismatched sizes.\n";
							continue;
						}

						// types will be treated as floats since that's what we use as glm types
						// check that arraySize matches expected size from uniform type
						size_t expectedSize = GLTypeSize(uinfo->type) / sizeof(float);

						if (arraySize != expectedSize) {
							std::cerr << "[Material JSON] Uniform array " << name << " element size mismatch, expected size " << expectedSize << ".\n";
							continue;
						}

						//convert each element to glm type and store in an array of that type
						switch (uinfo->type) {
						case GL_FLOAT_VEC2:
						case GL_INT_VEC2:
						case GL_BOOL_VEC2:
							PARSE_GLM_ARRAY(glm::vec2);
							break;
						case GL_FLOAT_VEC3:
						case GL_INT_VEC3:
						case GL_BOOL_VEC3:
							PARSE_GLM_ARRAY(glm::vec3);
							break;
						case GL_FLOAT_VEC4:
						case GL_INT_VEC4:
						case GL_BOOL_VEC4:
							PARSE_GLM_ARRAY(glm::vec4);
							break;
						case GL_FLOAT_MAT2:
							PARSE_GLM_ARRAY(glm::mat2);
							break;
						case GL_FLOAT_MAT3:
							PARSE_GLM_ARRAY(glm::mat3);
							break;
						case GL_FLOAT_MAT4:
							PARSE_GLM_ARRAY(glm::mat4);
							break;
						case GL_FLOAT_MAT2x3:
							PARSE_GLM_ARRAY(glm::mat2x3);
							break;
						case GL_FLOAT_MAT3x2:
							PARSE_GLM_ARRAY(glm::mat3x2);
							break;
						case GL_FLOAT_MAT2x4:
							PARSE_GLM_ARRAY(glm::mat2x4);
							break;
						case GL_FLOAT_MAT4x2:
							PARSE_GLM_ARRAY(glm::mat4x2);
							break;
						case GL_FLOAT_MAT3x4:
							PARSE_GLM_ARRAY(glm::mat3x4);
							break;
						case GL_FLOAT_MAT4x3:
							PARSE_GLM_ARRAY(glm::mat4x3);
							break;
						default:
							std::cerr << "[Material JSON] Uniform array " << name << " has unsupported type for vector/matrix array.\n";
							break;
						}
					}
				}
			}
		}
	}

	// load textures
	if (j.contains("textures") && j["textures"].is_object()) {
		for (auto& [name, val] : j["textures"].items())
		{
			// check that texture exists in material
			if (!mat.textures.contains(name)) {
				std::cerr << "[Material JSON] Material has no texture uniform: " << name << "\n";
				continue;
			}

			if (!val.is_string()) {
				std::cerr << "[Material JSON] Texture value for " << name << " is not a string.\n";
				continue;
			}

			// get texture handle from texture manager by name
			// texture should be already loaded by the texture manager before the material manager starts loading materials
			// and its name should be its path relative to the resoruces/textures/ directory
			std::string texturePath = val.get<std::string>();
			mat.setTexture(name, texturePath);
		}
	}

	// load local UBOs
	if (j.contains("ubos") && j["ubos"].is_object()) {
		for (auto& [blockName, val] : j["ubos"].items())
		{
			// check that UBO exists in material
			UboWriter* uboWriter = mat.GetLocalUboWriter(blockName);
			if (!uboWriter) {
				std::cerr << "[Material JSON] Material has no local UBO: " << blockName << "\n";
				continue;
			}
			if (!val.is_object()) {
				std::cerr << "[Material JSON] UBO value for " << blockName << " is not an object.\n";
				continue;
			}

			auto* writer = mat.GetLocalUboWriter(blockName);
			auto* blockInfo = writer->GetUboInfo();

			// set fields
			for (auto& [fieldName, fieldVal] : val.items())
			{
				const UniformBlockFieldInfo* finfo = blockInfo->GetField(fieldName);
				if (!finfo) {
					std::cerr << "[Material JSON] UBO " << blockName << " has no field: " << fieldName << "\n";
					continue;
				}

				// Scalar float / int / bool
				if (fieldVal.is_number_float()) {
					writer->Set(fieldName, fieldVal.get<float>());
				}
				else if (fieldVal.is_number_integer()) {
					writer->Set(fieldName, fieldVal.get<int>());
				}
				// Array or vector/matrix
				else if (fieldVal.is_array()) {
					// check if field is array
					if (finfo->size == 1) {
						// not an array, must be vector/matrix
						// check that size matches expected size from uniform type
						size_t expectedSize = GLTypeSize(finfo->type) / sizeof(float);
						if (fieldVal.size() != expectedSize) {
							std::cerr << "[Material JSON] UBO field " << fieldName << " size mismatch, expected size " << expectedSize << ".\n";
							continue;
						}

						// types will be treated as floats since that's what we use as glm types
						std::vector<float> floatValues;
						for (const auto& elem : val) {
							floatValues.push_back(elem.get<float>());
						}

						// convert to appropriate glm type based on uniform type
						switch (finfo->type) {
						case GL_FLOAT_VEC2:
						case GL_INT_VEC2:
						case GL_BOOL_VEC2:
							PARSE_GLM_SINGLE_WRITER(glm::vec2);
							break;
						case GL_FLOAT_VEC3:
						case GL_INT_VEC3:
						case GL_BOOL_VEC3:
							PARSE_GLM_SINGLE_WRITER(glm::vec3);
							break;
						case GL_FLOAT_VEC4:
						case GL_INT_VEC4:
						case GL_BOOL_VEC4:
							PARSE_GLM_SINGLE_WRITER(glm::vec4);
							break;
						case GL_FLOAT_MAT2:
							PARSE_GLM_SINGLE_WRITER(glm::mat2);
							break;
						case GL_FLOAT_MAT3:
							PARSE_GLM_SINGLE_WRITER(glm::mat3);
							break;
						case GL_FLOAT_MAT4:
							PARSE_GLM_SINGLE_WRITER(glm::mat4);
							break;
						case GL_FLOAT_MAT2x3:
							PARSE_GLM_SINGLE_WRITER(glm::mat2x3);
							break;
						case GL_FLOAT_MAT3x2:
							PARSE_GLM_SINGLE_WRITER(glm::mat3x2);
							break;
						case GL_FLOAT_MAT2x4:
							PARSE_GLM_SINGLE_WRITER(glm::mat2x4);
							break;
						case GL_FLOAT_MAT4x2:
							PARSE_GLM_SINGLE_WRITER(glm::mat4x2);
							break;
						case GL_FLOAT_MAT3x4:
							PARSE_GLM_SINGLE_WRITER(glm::mat3x4);
							break;
						case GL_FLOAT_MAT4x3:
							PARSE_GLM_SINGLE_WRITER(glm::mat4x3);
							break;
						default:
							std::cerr << "[Material JSON] UBO field " << fieldName << " has unsupported type for vector/matrix.\n";
							break;
						}
					}
					else if (finfo->size > 1) {
						// check if json array has the right number of elements
						if (val.size() != static_cast<size_t>(finfo->size)) {
							std::cerr << "[Material JSON] UBO field array " << fieldName << " size mismatch, expected size " << finfo->size << ".\n";
							continue;
						}

						// check if all elements are of the same type: int/bool, float or array(vector/matrix)
						bool allInt = true;
						bool allFloat = true;
						bool allArray = true;
						for (const auto& elem : fieldVal) {
							if (!elem.is_number_integer()) allInt = false;
							if (!elem.is_number_float()) allFloat = false;
							if (!elem.is_array()) allArray = false;
						}

						if (!allInt && !allFloat && !allArray) {
							std::cerr << "[Material JSON] UBO field array " << fieldName << " has mixed element types.\n";
							continue;
						}

						if (allInt) {
							if (!IsIntegerType(finfo->type)) {
								std::cerr << "[Material JSON] UBO field array " << fieldName << " type mismatch, expected integer type.\n";
								continue;
							}

							std::vector<int> intValues;
							for (const auto& elem : fieldVal) {
								intValues.push_back(elem.get<int>());
							}
							writer->SetArray(fieldName, intValues);
						}
						else if (allFloat) {
							if (!IsFloatType(finfo->type)) {
								std::cerr << "[Material JSON] UBO field array " << fieldName << " type mismatch, expected float type.\n";
								continue;
							}

							std::vector<float> floatValues;
							for (const auto& elem : fieldVal) {
								floatValues.push_back(elem.get<float>());
							}
							writer->SetArray(fieldName, floatValues);
						}
						else if (allArray) {
							// first check that all of them have the same size
							size_t arraySize = fieldVal[0].size();
							bool sizeMismatch = false;
							for (const auto& elem : fieldVal) {
								if (elem.size() != arraySize) {
									sizeMismatch = true;
									break;
								}
							}

							if (sizeMismatch) {
								std::cerr << "[Material JSON] UBO field array " << fieldName << " has elements with mismatched sizes.\n";
								continue;
							}

							// types will be treated as floats since that's what we use as glm types
							// check that arraySize matches expected size from uniform type
							size_t expectedSize = GLTypeSize(finfo->type) / sizeof(float);

							if (arraySize != expectedSize) {
								std::cerr << "[Material JSON] UBO field array " << fieldName << " element size mismatch, expected size " << expectedSize << ".\n";
								continue;
							}

							//convert each element to glm type and store in an array of that type
							switch (finfo->type) {
							case GL_FLOAT_VEC2:
							case GL_INT_VEC2:
							case GL_BOOL_VEC2:
								PARSE_GLM_ARRAY_WRITER(glm::vec2);
								break;
							case GL_FLOAT_VEC3:
							case GL_INT_VEC3:
							case GL_BOOL_VEC3:
								PARSE_GLM_ARRAY_WRITER(glm::vec3);
								break;
							case GL_FLOAT_VEC4:
							case GL_INT_VEC4:
							case GL_BOOL_VEC4:
								PARSE_GLM_ARRAY_WRITER(glm::vec4);
								break;
							case GL_FLOAT_MAT2:
								PARSE_GLM_ARRAY_WRITER(glm::mat2);
								break;
							case GL_FLOAT_MAT3:
								PARSE_GLM_ARRAY_WRITER(glm::mat3);
								break;
							case GL_FLOAT_MAT4:
								PARSE_GLM_ARRAY_WRITER(glm::mat4);
								break;
							case GL_FLOAT_MAT2x3:
								PARSE_GLM_ARRAY_WRITER(glm::mat2x3);
								break;
							case GL_FLOAT_MAT3x2:
								PARSE_GLM_ARRAY_WRITER(glm::mat3x2);
								break;
							case GL_FLOAT_MAT2x4:
								PARSE_GLM_ARRAY_WRITER(glm::mat2x4);
								break;
							case GL_FLOAT_MAT4x2:
								PARSE_GLM_ARRAY_WRITER(glm::mat4x2);
								break;
							case GL_FLOAT_MAT3x4:
								PARSE_GLM_ARRAY_WRITER(glm::mat3x4);
								break;
							case GL_FLOAT_MAT4x3:
								PARSE_GLM_ARRAY_WRITER(glm::mat4x3);
								break;
							default:
								std::cerr << "[Material JSON] UBO field array " << fieldName << " has unsupported type for vector/matrix array.\n";
								break;
							}
						}
					}
				}
			}
		}
	}
}

// =========================================================
// MaterialManager
// =========================================================
MaterialManager::MaterialHandle MaterialManager::LoadFromJSON(const std::string& JSONFilePath) {
	auto [mat, name, error] = policy.CreateFromJSON(JSONFilePath);
	if (!error.empty()) {
		if (error == "MaterialExists") {
			// Material already exists, return existing handle
			// Extra data from the json that might want to set uniforms/textures will be ignored
			return this->GetHandle(name);
		}

		std::cerr << "MaterialManager::LoadFromJSON: Failed to create material from JSON " << JSONFilePath << ": " << error << std::endl;
		return Handle{};
	}
	
	uint32_t id = nextID++;
	uint32_t gen = 1;

	mat.alive = true;
	resources[id] = { mat, gen };
	nameToHandle[name] = Handle{ id, gen };
	handleToName[id] = name;

	return Handle{ id, gen };
}

void MaterialManager::PreloadResources(const std::string& resourceDirectory) {
	std::string materialsDir = "materials/";
	std::filesystem::path fullDir = std::filesystem::path(resourceDirectory) / materialsDir;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(fullDir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			std::string fullPath = entry.path().string();
			std::string relativePath = std::filesystem::relative(entry.path(), fullDir).string();
			std::cout << "Loading material: " << relativePath << "\n";
			LoadFromJSON(fullPath);
		}
	}
}
