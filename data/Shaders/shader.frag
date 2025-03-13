#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in flat int texId;
layout(location = 5) in flat vec3 inViewPos;

layout(location = 0) out vec4 outColor;

//Material uniform
layout(set = 0, binding = 1) uniform MaterialData
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;
struct Light
{
    vec3    position;
    vec3    color;
    vec3    direction;
    int     type;
    float   intensity;
    float   radius;
};
layout(set = 0, binding = 2) uniform LightDataArray
{
    Light lights[10];
} lightArray;

//Texture uniforms
layout(set = 1, binding = 0) uniform sampler mSampler;
layout(set = 2, binding = 0) uniform texture2D mTextures[];

void main() 
{
    Light curLight = lightArray.lights[0];
    vec3 nrm = normalize(fragNormal);
    vec3 lightDir = normalize(curLight.position - fragPos);
    float diff = max(dot(nrm, lightDir), 0.0f);
    vec3 diffColor = diff * curLight.color;
    vec3 amb = material.ambient;
    vec3 viewDir = normalize(inViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, nrm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specColor = spec * curLight.color;
    vec3 lightCoeff = amb + diffColor + specColor;
    outColor = vec4(lightCoeff, 1.0f) * texture(sampler2D(mTextures[nonuniformEXT(texId)], mSampler), fragTexCoord); 
}