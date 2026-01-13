#version 460
#extension GL_ARB_shading_language_include : require
#include </defs.glsl> //! #include "../defs.glsl"

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec2 in_TexCoord;
layout(location = INSTANCE_MODEL_MATRIX) in mat4 in_instanceMatrix;

out vec4 gl_Position; 
out vec2 ex_TexCoord;

layout (std140) uniform Camera {
	FIXED_VEC3 viewPos;
	mat4 view;
	mat4 projection;
};

void main ()
{
	 // Camera basis vectors (world space)
    vec3 camRight   = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp      = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 camForward = -vec3(view[0][2], view[1][2], view[2][2]); 

    // World position of particle center
    vec3 worldPos = vec3(in_instanceMatrix[3]);

    // Extract emitter forward (using +Z column)
    vec3 emitterForward = normalize(vec3(in_instanceMatrix[1]));

    // Project emitter forward onto camera plane
    vec3 projected = emitterForward - dot(emitterForward, camForward) * camForward;

    // Handle degenerate case (emitter forward parallel to camForward)
    if (length(projected) < 0.0001)
        projected = camRight;

    projected = normalize(projected);

    // Construct rotated billboard basis
    vec3 billboardUp = projected;
    vec3 billboardRight = normalize(cross(camForward, billboardUp));

    // Extract non-uniform scale from instance matrix
    vec3 scale = vec3(
        length(vec3(in_instanceMatrix[0])),
        length(vec3(in_instanceMatrix[1])),
        length(vec3(in_instanceMatrix[2]))
    );

    // Apply scale & build final billboard vertex
    vec3 billboardPos =
        worldPos +
        billboardRight * in_Position.x * scale.x +
        billboardUp    * in_Position.y * scale.y;

    gl_Position = projection * view * vec4(billboardPos, 1.0);
    ex_TexCoord = in_TexCoord;
}