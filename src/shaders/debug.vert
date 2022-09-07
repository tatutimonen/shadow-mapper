#version 460 core

//------------------------------------------------------------------------
// Constants.

const uint INSTANCE_ID = gl_BaseInstance + gl_InstanceID;

//------------------------------------------------------------------------
// Vertex attributes.

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_nrm;
layout (location = 2) in vec2 a_tex;

//------------------------------------------------------------------------
// Outputs.

out VERT_OUT {
    vec2 tex;
    flat uint instanceID;
} VertOut;

//------------------------------------------------------------------------
// Uniforms etc.

layout (binding = 0, std140) uniform Camera {
    mat3x4 VT;
    mat4 P;
} u_camera;

layout (binding = 1, std430) buffer Model {
    mat3x4 MT[];
} b_model;

//------------------------------------------------------------------------

void main()
{
    VertOut.tex = a_tex;
    VertOut.instanceID = INSTANCE_ID;
    vec3 modelPos = vec4(a_pos, 1.0) * b_model.MT[INSTANCE_ID];
    vec3 viewModel = vec4(modelPos, 1.0) * u_camera.VT;
    gl_Position = u_camera.P * vec4(viewModel, 1.0);
}

//------------------------------------------------------------------------
