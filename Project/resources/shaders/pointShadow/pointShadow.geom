#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (std140) uniform Shadow {
	mat4 LightSpace[6];
    vec4 LightPos;
    float CascadedSplits[6];
};

out vec4 FragPos;

void main(void){
	for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; 
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = LightSpace[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}
