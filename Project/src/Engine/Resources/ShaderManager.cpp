#include "Engine/Resources/ShaderManager.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "Engine/Resources/ResourceManager.h"

// =========================================================
// Shader
// =========================================================
ShaderManager* Shader::_sm = nullptr;

void Shader::Bind() const
{
	//check if already bound
    if (program != Shader::_sm->currentProgram) {
        Shader::_sm->currentProgram = program;
        glUseProgram(program);
	}
}
void Shader::SetRaw(const std::string& name, GLenum GLtype, const void* data, size_t elementCount)
{
    auto it = reflection.uniforms.find(name);
    if (it == reflection.uniforms.end()) {
        return;
    }
    const UniformInfo& info = it->second;
    if (info.location == -1) {
        // uniform optimized out or part of UBO
        return;
    }
    if(info.type != GLtype) {
        std::cerr << "Type mismatch in SetRaw for uniform " << name << ": expected "
                  << GLTypeToString(info.type) << ", got " << GLTypeToString(GLtype) << std::endl;
        return;
	}

	auto loc = info.location;

    Bind();
    // Dispatch based on type
    switch (GLtype){
        // floats
    case GL_FLOAT:
        glUniform1fv(loc, (GLsizei)elementCount, (const float*)data);
        break;

    case GL_FLOAT_VEC2:
        glUniform2fv(loc, (GLsizei)elementCount, (const float*)data);
        break;

    case GL_FLOAT_VEC3:
        glUniform3fv(loc, (GLsizei)elementCount, (const float*)data);
        break;

    case GL_FLOAT_VEC4:
        glUniform4fv(loc, (GLsizei)elementCount, (const float*)data);
        break;

        // integers
    case GL_INT:
    case GL_BOOL:
        glUniform1iv(loc, (GLsizei)elementCount, (const int*)data);
        break;

    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
        glUniform2iv(loc, (GLsizei)elementCount, (const int*)data);
        break;

    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
        glUniform3iv(loc, (GLsizei)elementCount, (const int*)data);
        break;

    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
        glUniform4iv(loc, (GLsizei)elementCount, (const int*)data);
        break;

		// matrices
    case GL_FLOAT_MAT2:
        glUniformMatrix2fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT3:
        glUniformMatrix3fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT4:
        glUniformMatrix4fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT2x3:
        glUniformMatrix2x3fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT3x2:
        glUniformMatrix3x2fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT2x4:
        glUniformMatrix2x4fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT4x2:
        glUniformMatrix4x2fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT3x4:
        glUniformMatrix3x4fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    case GL_FLOAT_MAT4x3:
        glUniformMatrix4x3fv(loc, (GLsizei)elementCount, GL_FALSE, (const float*)data);
        break;

    default:
        break;
    }
}

// =========================================================
// ShaderPolicy
// =========================================================

// --- Shader Compilation Helpers ---
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

// --- Reflection Helpers ---
void ShaderPolicy::ReflectUniforms(GLuint program, ShaderReflection& out)
{
    GLint count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

    char nameBuffer[256];

    for (GLint i = 0; i < count; ++i) {
        GLsizei length;
        GLint size;     // array size
        GLenum type;

        glGetActiveUniform(program, i, sizeof(nameBuffer), &length, &size, &type, nameBuffer);

        std::string name(nameBuffer, length);
        std::string baseName;

        // OpenGL reports array uniforms as: "myArray[0]"
        // Normalize by stripping "[0]".
        if (name.size() > 3 && name.substr(name.size() - 3) == "[0]") {
            baseName = name.substr(0, name.size() - 3);
        }
        else {
			baseName = name;
        }

        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
            continue; // part of a UBO or optimized out

        UniformInfo info;
        info.name = name;
		info.baseName = baseName;
        info.type = type;
        info.size = size;
        info.location = location;

        out.uniforms[baseName] = info;
    }
}

void ShaderPolicy::ReflectUniformBlocks(GLuint program, ShaderReflection& out)
{
	auto& _ub = ResourceManager::Get().ubos;

    GLint blockCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);

    char nameBuffer[256];

    for (GLint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        GLsizei length;
        glGetActiveUniformBlockName(program, blockIndex, sizeof(nameBuffer), &length, nameBuffer);

        std::string blockName(nameBuffer, length);

        UniformBlockInfo blockInfo;
        blockInfo.name = blockName;
        blockInfo.index = blockIndex;

        GLint dataSize;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
        blockInfo.dataSize = dataSize;

        GLint blockSize;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
		blockInfo, dataSize = blockSize;

        // reflect internal fields
        GLint activeUniforms;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniforms);

        std::vector<GLint> indices(activeUniforms);
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());

        // Convert to GLuint for OpenGL API
        std::vector<GLuint> uindices(indices.begin(), indices.end());

        std::vector<GLint> offsets(activeUniforms);
        std::vector<GLint> types(activeUniforms);
        std::vector<GLint> sizes(activeUniforms);
        std::vector<GLint> arrayStrides(activeUniforms);
        std::vector<GLint> matrixStrides(activeUniforms);
        std::vector<GLint> rowMajors(activeUniforms);

        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_OFFSET, offsets.data());
        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_TYPE, types.data());
        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_SIZE, sizes.data());
        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_ARRAY_STRIDE, arrayStrides.data());
        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_MATRIX_STRIDE, matrixStrides.data());
        glGetActiveUniformsiv(program, activeUniforms, uindices.data(), GL_UNIFORM_IS_ROW_MAJOR, rowMajors.data());

        for (int i = 0; i < activeUniforms; i++) {
            char uniformName[256];
            GLsizei len;
            glGetActiveUniformName(program, indices[i], sizeof(uniformName), &len, uniformName);

			std::string name = std::string(uniformName, len);

            // OpenGL reports array uniforms as: "myArray[0]"
            // Normalize by stripping "[0]".
            if (name.size() > 3 && name.substr(name.size() - 3) == "[0]") {
                name = name.substr(0, name.size() - 3);
            }

            UniformBlockFieldInfo field;
            field.name = name;
            field.type = types[i];
            field.size = sizes[i];
            field.offset = offsets[i];
            field.arrayStride = arrayStrides[i];
            field.matrixStride = matrixStrides[i];
            field.rowMajor = (rowMajors[i] != 0);

            blockInfo.fields.push_back(field);
        }

        // Obtain binding point and bufferIndex from UboManager
        auto uboHandle = _ub.CreateOrGet(blockInfo);
        auto* ubo = _ub.Get(uboHandle);
        blockInfo.binding = ubo->binding;
        blockInfo.bufferID = ubo->bufferID;

        // bind the block to the binding point
        glUniformBlockBinding(program, blockIndex, blockInfo.binding);

        out.uniformBlocks[blockName] = blockInfo;
    }
}

void ShaderPolicy::Reflect(GLuint program, ShaderReflection& out)
{
    ReflectUniforms(program, out);
	ReflectUniformBlocks(program, out);
}

// --- Policy interface ---
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

	glDeleteShader(vs);
	glDeleteShader(fs);

    Shader shader;
    shader.program = program;
    shader.alive = true;

	Reflect(program, shader.reflection);

    //temp debug print
	std::cout << "Shader Reflection for " << name << ":\n";
    for (auto& [name, info] : shader.reflection.uniforms) {
		std::cout << "  Uniform: " << name << " type=" << GLTypeToString(info.type)
            << " size=" << info.size << " location=" << info.location << "\n";
    }
    for (auto& [name, block] : shader.reflection.uniformBlocks) {
        std::cout <<"  Uniform Block: " << name << " index=" << block.index
			<< " binding=" << block.binding << " bufferId=" << block.bufferID << " dataSize=" << block.dataSize << "\n";
        for(auto& field : block.fields) {
            std::cout << "    Field: " << field.name << " type=" << GLTypeToString(field.type)
                << " size=" << field.size << " offset=" << field.offset
                << " arrayStride=" << field.arrayStride << " matrixStride=" << field.matrixStride
                << " rowMajor=" << field.rowMajor << "\n";
		}
    }

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
}

// =========================================================
// ShaderManager
// =========================================================
ShaderManager::ShaderManager()
{
	// Give Shader pointer to itself for currentProgram access
	Shader::_sm = this;
}

void ShaderManager::UseShader(const ShaderHandle& h)
{
    const Shader* shader = Get(h);
    if (shader && shader->program != currentProgram) {
		currentProgram = shader->program;
        glUseProgram(shader->program);
    }
}

void ShaderManager::UseShader(const Shader& shader)
{
    if (shader.program != currentProgram) {
        currentProgram = shader.program;
        glUseProgram(shader.program);
    }
}

void ShaderManager::UseShader(const std::string& name) {
    const Shader* shader = Get(name);
    if (shader && shader->program != currentProgram) {
        currentProgram = shader->program;
        glUseProgram(shader->program);
    }
}