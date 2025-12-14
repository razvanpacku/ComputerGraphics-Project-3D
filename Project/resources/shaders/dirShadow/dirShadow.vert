#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (location = 0) in vec4 in_Position;
layout(location = INSTANCE_MODEL_MATRIX) in mat4 in_instanceMatrix;

out vec4 gl_Position; 
void main ()
{
	gl_Position = in_instanceMatrix * in_Position;
}