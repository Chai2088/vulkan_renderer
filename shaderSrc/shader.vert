#version 450

layout(push_constant) uniform PushConstants
{
    mat4 model;
    int texId;
} pushConstants;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

// Model matrix (comes in as four vec4s)
layout(location = 3) in vec4 row0;
layout(location = 4) in vec4 row1;
layout(location = 5) in vec4 row2;
layout(location = 6) in vec4 row3;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat int texId;

void main() 
{
    mat4 modelMatrix = mat4(row0, row1, row2, row3);
    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    texId = pushConstants.texId;
}