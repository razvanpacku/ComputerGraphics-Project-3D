// Shader-ul de fragment / Fragment shader 
#version 460

in vec4 ex_Color;
in vec2 ex_TexCoord;

out vec4 out_Color;

uniform sampler2D Texture;

layout (std140) uniform Test {
	mat4 first;
	vec3 testColor[2];
};

void main(void){
	vec4 TexColor = texture(Texture, ex_TexCoord);

	//mix the testColors based on the v texture coordinate
	vec4 mixColor = vec4(mix(testColor[0], testColor[1], ex_TexCoord.y), 1.0f);

	out_Color = ex_Color * TexColor * mixColor;
}