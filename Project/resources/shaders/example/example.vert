// Shader-ul de varfuri / Vertex shader
#version 460

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec4 in_Color;
layout (location = 2) in vec2 in_TexCoord;

out vec4 gl_Position; 
out vec4 ex_Color;
out vec2 ex_TexCoord;

uniform mat4 rotation;

void main ()
{
   gl_Position = rotation * in_Position;
   ex_Color = in_Color;
   ex_TexCoord = in_TexCoord;
}