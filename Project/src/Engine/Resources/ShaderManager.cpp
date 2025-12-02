#include "Engine/Resources/ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

// =========================================================
// Shader
// =========================================================
GLint Shader::GetUniformLocation(const std::string& name) const
{
    auto it = uniformCache.find(name);
    if (it != uniformCache.end())
        return it->second;

    GLint location = glGetUniformLocation(program, name.c_str());
    uniformCache[name] = location;
    return location;
}

// --- Uniform Setters ---
void Shader::SetUniform(const std::string& name, int value) const
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform(const std::string& name, float value) const
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform(const std::string& name, const glm::vec2& v) const
{
    glUniform2fv(GetUniformLocation(name), 1, &v[0]);
}

void Shader::SetUniform(const std::string& name, const glm::vec3& v) const
{
    glUniform3fv(GetUniformLocation(name), 1, &v[0]);
}

void Shader::SetUniform(const std::string& name, const glm::vec4& v) const
{
    glUniform4fv(GetUniformLocation(name), 1, &v[0]);
}

void Shader::SetUniform(const std::string& name, const glm::mat4& m) const
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &m[0][0]);
}

void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const
{
    GLuint index = glGetUniformBlockIndex(program, blockName.c_str());
    if (index != GL_INVALID_INDEX)
        glUniformBlockBinding(program, index, bindingPoint);
}

// =========================================================
// ShaderPolicy
// =========================================================
std::string ShaderPolicy::LoadFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint ShaderPolicy::Compile(GLenum type, const std::string& src)
{
    GLuint shader = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }

    return shader;
}

GLuint ShaderPolicy::LinkProgram(GLuint vs, GLuint fs)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Shader ShaderPolicy::Create(const std::string& name, const ShaderResourceInfo& shaderInfo)
{
    std::string vsSrc = LoadFile(shaderInfo.vertexPath);
    std::string fsSrc = LoadFile(shaderInfo.fragmentPath);
    GLuint vs = Compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = Compile(GL_FRAGMENT_SHADER, fsSrc);
    GLuint program = LinkProgram(vs, fs);

    if(program == 0)
    {
        std::cerr << "Failed to create shader program for " << name << std::endl;
	}

    Shader shader;
    shader.program = program;
    shader.alive = true;
    return shader;
}

void ShaderPolicy::Destroy(Shader& shader)
{
    if (shader.program != 0)
    {
        glDeleteProgram(shader.program);
        shader.program = 0;
    }
    shader.alive = false;
    shader.uniformCache.clear();
}

void ShaderManager::UseShader(const ShaderHandle& h) const
{
    const Shader* shader = Get(h);
    if (shader)
        glUseProgram(shader->program);
}

void ShaderManager::UseShader(const Shader& shader) const
{
    glUseProgram(shader.program);
}

void ShaderManager::UseShader(const std::string& name) {
    const Shader* shader = Get(name);
    if (shader) {
        glUseProgram(shader->program);
    }
}