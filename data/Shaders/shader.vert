#version 450

layout(push_constant) uniform PushConstants
{
    mat4 model;
    int matIdx;
} pushConstants;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
    mat4 depthMVP;
    vec3 viewPos;
    int lightCount;
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
layout(location = 4) out vec4 lightFragPos;
layout(location = 5) out flat int matIdx;
layout(location = 6) out flat int lightCount;
layout(location = 7) out flat vec3 outViewPos;

void main() 
{

    mat4 model = mat4(row0, row1, row2, row3);
    gl_Position = ubo.proj * (ubo.view * (model * vec4(inPosition, 1.0)));
    fragNormal = mat3(transpose(inverse(model))) * inNormal;
    fragColor = inColor;
    
    fragPos = (model * vec4(inPosition, 1.0)).xyz;
    const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
    lightFragPos = biasMat * ubo.depthMVP * vec4(fragPos, 1.0f);
    
    outViewPos = ubo.viewPos;
    fragTexCoord = inTexCoord;
    matIdx = pushConstants.matIdx;
    lightCount = ubo.lightCount;
}