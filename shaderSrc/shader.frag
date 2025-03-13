#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int texId;

layout(location = 0) out vec4 outColor;

//Material uniform
layout(set = 0, binding = 1) uniform MaterialData
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

//Texture uniforms
layout(set = 1, binding = 0) uniform sampler mSampler;
layout(set = 2, binding = 0) uniform texture2D mTextures[];

void main() 
{
    outColor = texture(sampler2D(mTextures[nonuniformEXT(texId)], mSampler), fragTexCoord);
}