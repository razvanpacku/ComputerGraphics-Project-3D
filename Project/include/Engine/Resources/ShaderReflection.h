#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <stdexcept>


/*
* Structures to hold reflection info about shader uniforms and uniform blocks.
*/


// UniformInfo holds information about a single uniform variable or array in a shader program.
struct UniformInfo {
    std::string name;
	std::string baseName;   // for arrays, name without [0]
	GLenum type;            // GL type of the uniform (e.g., GL_FLOAT, GL_INT, etc.)
    GLint size;             // array size (1 for non-array)
    GLint location;         // obtained from glGetUniformLocation
    GLint elementStride = 1;
};

// UniformBlockFieldInfo holds information about a single field within a uniform block.
struct UniformBlockFieldInfo {
    std::string name;
    GLenum type;
    GLint size;             // array length (1 for non-array)
    GLint offset;           // byte offset in UBO
    GLint arrayStride;      // for arrays
    GLint matrixStride;     // for matrices
    bool rowMajor;
};

// UniformBlockInfo holds information about a uniform block in a shader program.
struct UniformBlockInfo {
    std::string name;
    GLuint index;                               // uniform block index
    GLuint binding;                             // binding point
	GLuint bufferID;	                        // OpenGL buffer ID
    GLint dataSize;                             // total required size for buffer
	std::vector<UniformBlockFieldInfo> fields;  // fields within the block
	int blockSize;                           // size of the block

	bool HasField(const std::string& fieldName) const;
	const UniformBlockFieldInfo* GetField(const std::string& fieldName) const;
};

// SamplerInfo holds information about a sampler uniform in a shader program.
struct SamplerInfo {
    std::string name;
    GLenum type;       // GL_SAMPLER_2D, GL_SAMPLER_CUBE, etc.
    GLint location;    // glGetUniformLocation
    GLint textureUnit; // assigned texture unit
};

// ShaderReflection holds all reflection information for a shader program.
class ShaderReflection {
public:
    std::unordered_map<std::string, UniformInfo> uniforms;
    std::unordered_map<std::string, UniformBlockInfo> uniformBlocks;
    std::unordered_map<std::string, SamplerInfo> samplers;

    bool HasUniform(const std::string& name) const;
	const UniformInfo* GetUniform(const std::string& name) const;

	bool HasBlock(const std::string& name) const;
	const UniformBlockInfo* GetBlock(const std::string& name) const;
};

// Util function to get string representation of GL types
std::string GLTypeToString(GLenum type);

//Util function to get size of GL types
size_t GLTypeSize(GLenum type);

bool IsIntegerType(GLenum type);
bool IsFloatType(GLenum type);
bool IsArrayType(GLenum type);
bool IsSamplerType(GLenum type);

// UniformValue stores a value of a Uniform through type erasure.
struct UniformValue {
	GLenum type;                // GL type of the uniform
	std::vector<uint8_t> data;  // raw byte data
	size_t elementCount = 1;    // number of elements (1 for non-array)

	bool dirty = false;

    template<typename T>
    void Set(const T& value, size_t index = 0) {
        size_t size = GLTypeSize(type);

		index = glm::clamp(index, size_t(0), elementCount - 1);

		data.resize(size);
		memcpy(data.data() + index * size, &value, size);
        dirty = true;
    }

    template<typename T>
    void SetArray(const T* arr) {
		size_t size = GLTypeSize(type) * elementCount;

        data.resize(size);
        memcpy(data.data(), arr, size);
		dirty = true;
    }
};

template<typename T>
concept GlmType = requires(T v) {
    glm::value_ptr(v);
};

// Utility function to convert a span of floats to a glm type G
template<GlmType G>
G glm_from_floats(std::vector<float> src) {
    static_assert(std::is_trivially_copyable_v<G>, "G must be trivially copyable");
    G out{}; // zero-initialize including padding
    // copy up to src.size() floats into the component storage
    size_t bytesToCopy = src.size() * sizeof(float);
    // value_ptr(out) points to the contiguous component floats (no padding)
    std::memcpy(glm::value_ptr(out), src.data(), bytesToCopy);
    return out;
}
