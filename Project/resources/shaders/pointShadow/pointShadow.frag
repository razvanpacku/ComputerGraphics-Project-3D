#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

in vec4 FragPos;

layout (std140) uniform Shadow {
	mat4 LightSpace[6];
    vec4 LightPos;
    float FarPlane;
};

out float gl_FragDepth;

void main(void){

	float dist = length(FragPos.xyz - LightPos.xyz);
	dist /= FarPlane;
	gl_FragDepth = dist;
}