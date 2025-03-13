#version 450

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec3 viewPos;
    int texId;
} pushConstants;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

// Model matrix (comes in as four vec4s)
layout(location = 4) in vec4 row0;
layout(location = 5) in vec4 row1;
layout(location = 6) in vec4 row2;
layout(location = 7) in vec4 row3;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out flat int texId;
layout(location = 5) out flat vec3 outViewPos;

void main() 
{
    gl_Position = ubo.proj * (ubo.view * (pushConstants.model * vec4(inPosition, 1.0)));
    fragPos = (pushConstants.model * vec4(inPosition, 1.0)).xyz;
    fragNormal = mat3(transpose(inverse(pushConstants.model))) * inNormal;
    fragColor = inColor;
    outViewPos = pushConstants.viewPos;
    fragTexCoord = inTexCoord;
    texId = pushConstants.texId;
}