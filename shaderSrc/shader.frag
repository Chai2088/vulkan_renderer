#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//Texture uniforms
layout(binding = 1) uniform texture2D mTexture;
layout(binding = 2) uniform sampler mSampler;
void main() {
    outColor = texture(sampler2D(mTexture, mSampler), fragTexCoord);
}