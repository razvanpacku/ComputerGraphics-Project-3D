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

bool UniformBlockInfo::HasField(const std::string& fieldName) const {
	for (const auto& field : fields) {
		if (field.name == fieldName) {
			return true;
		}
	}
	return false;
}

const UniformBlockFieldInfo* UniformBlockInfo::GetField(const std::string& name) const {
	for (const auto& field : fields) {
		if (field.name == name) {
			return &field;
		}
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

size_t GLTypeSize(GLenum type) {
	switch (type) {
	case GL_FLOAT:			return sizeof(float); break;
	case GL_FLOAT_VEC2:		return sizeof(float) * 2; break;
	case GL_FLOAT_VEC3:		return sizeof(float) * 3; break;
	case GL_FLOAT_VEC4:		return sizeof(float) * 4; break;

	case GL_INT:
	case GL_BOOL:           return sizeof(int); break;
	case GL_INT_VEC2:
	case GL_BOOL_VEC2:		return sizeof(int) * 2; break;
	case GL_INT_VEC3:
	case GL_BOOL_VEC3:		return sizeof(int) * 3; break;
	case GL_INT_VEC4:
	case GL_BOOL_VEC4:		return sizeof(int) * 4; break;

	case GL_FLOAT_MAT2:		return sizeof(float) * 4; break;
	case GL_FLOAT_MAT3:		return sizeof(float) * 9; break;
	case GL_FLOAT_MAT4:		return sizeof(float) * 16; break;
	case GL_FLOAT_MAT2x3:	return sizeof(float) * 6; break;
	case GL_FLOAT_MAT2x4:   return sizeof(float) * 8; break;
	case GL_FLOAT_MAT3x2:   return sizeof(float) * 6; break;
	case GL_FLOAT_MAT3x4:   return sizeof(float) * 12; break;
	case GL_FLOAT_MAT4x2:   return sizeof(float) * 8; break;
	case GL_FLOAT_MAT4x3:   return sizeof(float) * 12; break;
	default:				return 0; break;
	}
}

bool IsIntegerType(GLenum type) {
	switch (type) {
	case GL_INT:
	case GL_BOOL:
		return true;
	default:
		return false;
	}
}

bool IsFloatType(GLenum type) {
	switch (type) {
	case GL_FLOAT:
		return true;
	default:
		return false;
	}
}

bool IsArrayType(GLenum type) {
	switch (type) {
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
	case GL_INT_VEC2:
	case GL_INT_VEC3:
	case GL_INT_VEC4:
	case GL_BOOL_VEC2:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC4:
	case GL_FLOAT_MAT2:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT4:
	case GL_FLOAT_MAT2x3:
	case GL_FLOAT_MAT2x4:
	case GL_FLOAT_MAT3x2:
	case GL_FLOAT_MAT3x4:
	case GL_FLOAT_MAT4x2:
	case GL_FLOAT_MAT4x3:
		return true;
	default:
		return false;
	}
}

bool IsSamplerType(GLenum type) {
	switch (type) {
	case GL_SAMPLER_2D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
		return true;
	default:
		return false;
	}
}