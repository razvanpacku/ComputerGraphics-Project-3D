#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (location = 0) in vec4 in_Position;
layout(location = INSTANCE_LAYOUT) in mat4 in_instanceMatrix;

out vec4 gl_Position; 

layout (std140) uniform Shadow {
	mat4 LightSpace[6];
    vec4 LightPos;
    float FarPlane;
};

void main ()
{
	gl_Position = LightSpace[0] * in_instanceMatrix * in_Position;
}