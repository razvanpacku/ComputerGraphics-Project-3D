#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec2 in_TexCoord;
layout(location = INSTANCE_UV_OFFSET) in vec4 in_UVOffset;
layout(location = INSTANCE_MODEL_MATRIX) in mat4 in_instanceMatrix;

out vec4 gl_Position; 
out vec2 ex_TexCoord;
out vec4 ex_UVOffset;

layout (std140) uniform GUICamera {
	mat4 view;
};

void main ()
{
	gl_Position = view * in_instanceMatrix * in_Position;
	ex_TexCoord = in_TexCoord;
	ex_UVOffset = in_UVOffset;
}