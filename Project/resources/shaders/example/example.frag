// Shader-ul de fragment / Fragment shader 
#version 460

in vec4 ex_Color;
in vec2 ex_TexCoord;

out vec4 out_Color;

uniform sampler2D Texture;

void main(void){
	vec4 TexColor = texture(Texture, ex_TexCoord);
	out_Color = ex_Color * TexColor;
}