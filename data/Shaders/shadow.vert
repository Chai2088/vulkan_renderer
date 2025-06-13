#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 shadowMVP;
}ubo;

layout(location = 0) in vec3 inPosition;

// Model matrix (comes in as four vec4s)
layout(location = 1) in vec4 row0;
layout(location = 2) in vec4 row1;
layout(location = 3) in vec4 row2;
layout(location = 4) in vec4 row3;


out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
    mat4 model = mat4(row0, row1, row2, row3);
    gl_Position = ubo.shadowMVP * (model * vec4(inPosition, 1.0f));
}