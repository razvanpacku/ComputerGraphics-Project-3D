#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

in vec2 ex_TexCoord;

out vec4 out_Color;

uniform sampler2D tex;

void main(void){
	vec4 TexColor = texture(tex, ex_TexCoord);

	out_Color = TexColor;
}