#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in flat int matIdx;
layout(location = 5) in flat int lightCount;
layout(location = 6) in flat vec3 inViewPos;

layout(location = 0) out vec4 outColor;

#define MAX_MATERIALS 100

//Material uniform
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float ior;
	int illum;

    int ambientTexIdx;
    int diffuseTexIdx;
    int specularTexIdx;
};
layout(set = 0, binding = 1) uniform MaterialData
{
    Material mats[MAX_MATERIALS];
} materialArray;
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

vec4 ComputeLightingValue(int lightIdx)
{
    Material curMat = materialArray.mats[matIdx];
    Light curLight = lightArray.lights[lightIdx];

    vec3 nrm = normalize(fragNormal);
    vec3 lightDir = normalize(curLight.position - fragPos);
    //Diffuse
    float diff = max(dot(nrm, lightDir), 0.0f);
    vec3 diffColor = diff * curMat.diffuse;
    //Ambient
    vec3 amb = curMat.ambient;
    //Specular
    vec3 viewDir = normalize(inViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, nrm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specColor = spec * curMat.specular;

    vec4 lightCoeff = vec4(0.0f);
    //Get the ambient, diffuse and specular maps
    if(curMat.ambientTexIdx != -1)
    {
        vec4 ambientTex = texture(sampler2D(mTextures[nonuniformEXT(curMat.ambientTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(amb, 1.0f) * ambientTex;
    }
    if(curMat.diffuseTexIdx != -1)
    {
        vec4 diffuseTex = texture(sampler2D(mTextures[nonuniformEXT(curMat.diffuseTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(diffColor, 1.0f) * diffuseTex;
    }
    if(curMat.specularTexIdx != -1)
    {
        vec4 specularTex = texture(sampler2D(mTextures[nonuniformEXT(curMat.specularTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(specColor, 1.0f) * specularTex;
    }

    return lightCoeff * vec4(curLight.color, 1.0f); 
}


void main() 
{
    if(matIdx != -1)
    {
        outColor = vec4(0.0f);
        for(int i = 0; i < lightCount; ++i)
        {
            outColor += ComputeLightingValue(i);
        }
    }
    else
        outColor = vec4(fragColor, 1.0f);
}