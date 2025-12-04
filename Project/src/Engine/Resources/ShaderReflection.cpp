#include "Engine/Resources/ShaderReflection.h"

// =========================================================
// ShaderReflection
// =========================================================
bool ShaderReflection::HasUniform(const std::string& name) const {
	return uniforms.contains(name);
}

const UniformInfo* ShaderReflection::GetUniform(const std::string& name) const {
	auto it = uniforms.find(name);
	if (it != uniforms.end()) {
		return &it->second;
	}
	return nullptr;
}

bool ShaderReflection::HasBlock(const std::string& name) const {
	return uniformBlocks.contains(name);
}

const UniformBlockInfo* ShaderReflection::GetBlock(const std::string& name) const {
	auto it = uniformBlocks.find(name);
	if (it != uniformBlocks.end()) {
		return &it->second;
	}
	return nullptr;
}

// Util function to get string representation of GL types
std::string GLTypeToString(GLenum type) {
	switch (type) {
		case GL_FLOAT: return "float";
		case GL_FLOAT_VEC2: return "vec2";
		case GL_FLOAT_VEC3: return "vec3";
		case GL_FLOAT_VEC4: return "vec4";
		case GL_INT: return "int";
		case GL_INT_VEC2: return "ivec2";
		case GL_INT_VEC3: return "ivec3";
		case GL_INT_VEC4: return "ivec4";
		case GL_BOOL: return "bool";
		case GL_BOOL_VEC2: return "bvec2";
		case GL_BOOL_VEC3: return "bvec3";
		case GL_BOOL_VEC4: return "bvec4";
		case GL_FLOAT_MAT2: return "mat2";
		case GL_FLOAT_MAT3: return "mat3";
		case GL_FLOAT_MAT4: return "mat4";
		case GL_SAMPLER_2D: return "sampler2D";
		case GL_SAMPLER_CUBE: return "samplerCube";
		default: return "unknown";
	}
}