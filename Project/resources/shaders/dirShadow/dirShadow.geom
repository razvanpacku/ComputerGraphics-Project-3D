#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (triangles, invocations = 6) in;
layout (triangle_strip, max_vertices=3) out;

layout (std140) uniform Shadow {
	mat4 LightSpace[6];
    vec4 LightPos;
    float CascadedSplits[6];
};

void main(){
    gl_Layer = gl_InvocationID;
    for(int i = 0; i < 3; ++i) // for each triangle vertex
    {
        gl_Position = LightSpace[gl_Layer] * gl_in[i].gl_Position;
        EmitVertex();
    }    
    EndPrimitive();
}