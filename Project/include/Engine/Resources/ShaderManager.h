#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>


#include "ResourceManagerTemplate.h"

struct Shader : public IResource {
    GLuint program = 0;
    mutable std::unordered_map<std::string, GLint> uniformCache;

    GLint GetUniformLocation(const std::string& name) const;
    void BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const;

    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, const glm::vec2& v) const;
    void SetUniform(const std::string& name, const glm::vec3& v) const;
    void SetUniform(const std::string& name, const glm::vec4& v) const;
    void SetUniform(const std::string& name, const glm::mat4& m) const;
};

struct ShaderResourceInfo {
    std::string vertexPath;
    std::string fragmentPath;
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
    GLuint LinkProgram(GLuint vs, GLuint fs);
};

class ShaderManager : public ResourceManagerTemplate<Shader, ShaderPolicy> {
public:
	using ShaderHandle = Handle;
    // using a shader
    void UseShader(const ShaderHandle& h) const;
	void UseShader(const Shader& shader) const;
	void UseShader(const std::string& name);
};

