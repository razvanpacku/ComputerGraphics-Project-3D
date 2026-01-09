#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "ResourceManagerTemplate.h"
#include "ShaderReflection.h"
#include "UniformSetter.h"

//forward declaration
class ShaderManager;
struct GLStateCache;

struct Shader : public IResource {
    GLuint program = 0;

    ShaderReflection reflection;

    void Bind() const;

    //returns false if uniform doesn't exist
    //index is used for array uniforms, gets clamped
    template<typename T>
    bool Set(const std::string& name, const T& value, GLint index = 0) const{
        // ensure uniform exists
        auto it = reflection.uniforms.find(name);
        if (it == reflection.uniforms.end()) {
            return false;
        }
        const UniformInfo& info = it->second;

        if (info.location == -1) {
            // uniform optimized out or part of UBO
            return false;
        }

		// clamp index to size
		GLint clampedIndex = glm::clamp(index, 0, info.size - 1);

		GLint location = info.location + clampedIndex * info.elementStride;

        Bind();

        // Dispatch the specialized uniform setter
        UniformSetter<T>::Apply(location, value);
        return true;
    }

    void SetRaw(const std::string& name, GLenum GLtype, const void* data, size_t elementCount);

private:
    // reference to the ShaderManager itself (to get access to the currentProgram)
	// this is done because getting the current program from OpenGL is slow
    static ShaderManager* _sm;

	friend class ShaderManager;
};

struct ShaderResourceInfo {
    std::string vertexPath;
    std::string fragmentPath;

	std::string geometryPath; // optional
};

class ShaderPolicy : public IResourcePolicy<Shader, ShaderResourceInfo> {
public:
    // These aliases are required by the ResourceManagerTemplate concepts to work
    using ResourceInfo = ShaderResourceInfo;
    using ResourceType = Shader;

    Shader Create(const std::string& name, const ShaderResourceInfo& resourceInfo) override;
    void Destroy(Shader& res) override;

private:
    std::string LoadFile(const std::string& path);
    GLuint Compile(GLenum type, const std::string& src);
	GLuint LinkProgram(GLuint vs, GLuint fs, GLuint gs = 0);

	// Reflection helpers
	static void ReflectUniforms(GLuint program, ShaderReflection& out);
	static void ReflectUniformBlocks(GLuint program, ShaderReflection& out);
	static void Reflect(GLuint program, ShaderReflection& out);

    static ShaderManager* _sm;

	friend class ShaderManager;
};

class ShaderManager : public ResourceManagerTemplate<Shader, ShaderPolicy> {
public:
	using ShaderHandle = Handle;

    ShaderManager();

	// Activate the shader
    void UseShader(const ShaderHandle& h, GLStateCache* glState = nullptr);
	void UseShader(const Shader& shader);
	void UseShader(const std::string& name, GLStateCache* glState = nullptr);

	virtual void PreloadResources(const std::string& resourceDirectory) override;

	bool HasInclude() const { return hasInclude; } 
	std::string GetDefineFileSource() const { return defineFileSource; }

	GLuint currentProgram = 0;
private:
	bool hasInclude = false;
    std::string defineFileSource;
};

