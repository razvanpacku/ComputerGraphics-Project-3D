#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>


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
};

// ShaderReflection holds all reflection information for a shader program.
class ShaderReflection {
public:
    std::unordered_map<std::string, UniformInfo> uniforms;
    std::unordered_map<std::string, UniformBlockInfo> uniformBlocks;

    bool HasUniform(const std::string& name) const;
	const UniformInfo* GetUniform(const std::string& name) const;

	bool HasBlock(const std::string& name) const;
	const UniformBlockInfo* GetBlock(const std::string& name) const;
};

// Util function to get string representation of GL types
std::string GLTypeToString(GLenum type);

