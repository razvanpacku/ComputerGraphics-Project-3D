#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec2 in_TexCoord;
layout (location = 2) in vec3 in_Normal;
layout(location = 3) in mat4 in_instanceMatrix;

out vec4 gl_Position; 
out vec4 ex_Color;
out vec2 ex_TexCoord;
out vec3 ex_Normal;

out vec3 fragPos;

layout (std140) uniform Camera {
	FIXED_VEC3 viewPos;
	mat4 view;
	mat4 projection;
};

void main ()
{
	fragPos = vec3(in_instanceMatrix * in_Position);
	gl_Position = projection * view * in_instanceMatrix * in_Position;
	ex_TexCoord = in_TexCoord;
	ex_Normal = mat3(transpose(inverse(in_instanceMatrix))) * in_Normal;
}