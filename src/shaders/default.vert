#version 460 core

//------------------------------------------------------------------------

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoords;

//------------------------------------------------------------------------

out VERT_OUT {
    vec2 texCoords;
} vert_out;

//------------------------------------------------------------------------

uniform mat4 u_VPMatrix;
uniform mat4 u_MMatrix;

//------------------------------------------------------------------------

void main()
{
    gl_Position = u_VPMatrix * u_MMatrix * vec4(a_position, 1.0);
    vert_out.texCoords = a_texCoords;
}

//------------------------------------------------------------------------
