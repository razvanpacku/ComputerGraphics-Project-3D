#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

in vec2 ex_TexCoord;
in vec4 ex_UVOffset;

out vec4 out_Color;

uniform sampler2D tex;
uniform bool hasBackground = false;
uniform vec4 backColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void main(void){
	vec4 TexColor = texture(tex, ex_TexCoord * ex_UVOffset.zw + ex_UVOffset.xy);
	if(hasBackground){
		out_Color = mix(backColor, TexColor, TexColor.a);
	}
	else{
		out_Color = TexColor;
	}
}