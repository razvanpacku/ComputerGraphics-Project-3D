#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

out vec4 out_Color;

uniform vec3 color;

void main(void){

	out_Color = vec4(color, 1.0f);
}