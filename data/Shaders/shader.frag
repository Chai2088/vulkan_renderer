#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec4 lightFragPos;
layout(location = 5) in flat int matIdx;
layout(location = 6) in flat int lightCount;
layout(location = 7) in flat vec3 inViewPos;

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
//Depth map
layout(set = 0, binding = 3) uniform sampler2D shadowMap;
//Texture uniforms
layout(set = 1, binding = 0) uniform sampler mSampler;
layout(set = 2, binding = 0) uniform texture2D mTextures[];

float bias = 0.005f;

float textureProj(vec4 shadowCoord, vec2 off, float bias)
{
    // Perform perspective division
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // Check if current fragment is outside light frustum
    if(projCoords.z > 1.0 || projCoords.x < -1.0 || projCoords.x > 1.0 
       || projCoords.y < -1.0 || projCoords.y > 1.0)
        return 0.0f;

    if ( projCoords.z > -1.0 && projCoords.z < 1.0 ) 
	{ 
        // Sample shadow map with offset
        float shadowDepth = texture(shadowMap, projCoords.xy + off).r;
        float curDepth = projCoords.z;
        if(shadowDepth < curDepth)
            return 1.0f;
    }
    return 0.0f;
}

float filterPCF(vec4 shadowCoord, float bias)
{
    ivec2 texDim = textureSize(shadowMap, 0);
    float scale = 1.0;
    float dx = scale / float(texDim.x);
    float dy = scale / float(texDim.y);

    float shadowFactor = 0.0;
    int range = 1;
    int count = 0;
    
    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            shadowFactor += textureProj(shadowCoord, vec2(dx*x, dy*y), bias);
            count++;
        }
    }
    
    return shadowFactor / float(count);
}

vec4 Phong(Material mat, Light light, vec3 lightDir, int type)
{
    vec3 nrm = normalize(fragNormal);
    vec3 viewDir = normalize(inViewPos - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    //Diffuse
    float diff = max(dot(nrm, lightDir), 0.0f);
    vec3 diffColor = diff * mat.diffuse;
    //Ambient
    vec3 amb = mat.ambient;
    //Specular
    vec3 reflectDir = reflect(-lightDir, nrm);
    float spec = pow(max(dot(nrm, halfDir), 0.0f), mat.shininess);
    vec3 specColor = spec * mat.specular;

    //Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (distance * distance);    

    vec4 lightCoeff = vec4(0.0f);
    //Get the ambient, diffuse and specular maps
    if(mat.ambientTexIdx != -1)
    {
        vec4 ambientTex = texture(sampler2D(mTextures[nonuniformEXT(mat.ambientTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(amb, 1.0f) * ambientTex;
    }
    if(mat.diffuseTexIdx != -1)
    {
        vec4 diffuseTex = texture(sampler2D(mTextures[nonuniformEXT(mat.diffuseTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(diffColor, 1.0f) * diffuseTex;
    }
    if(mat.specularTexIdx != -1)
    {
        vec4 specularTex = texture(sampler2D(mTextures[nonuniformEXT(mat.specularTexIdx)], mSampler), fragTexCoord);
        lightCoeff += vec4(specColor, 1.0f) * specularTex;
    }
    //Check if the light is directional, if so dont apply attenuation
    if(type == 1)
    {
        //Perform perspective division and offset the projection coordinate to [0,1]
        float shadow = filterPCF(lightFragPos, bias);

        vec4 color = (1.0f - shadow) * light.intensity * lightCoeff * vec4(light.color, 1.0f);
        color.w = 1.0f;
        return color; 
    }
    //If its spotlight or point light apply attenuation
    return attenuation * light.intensity * lightCoeff * vec4(light.color, 1.0f); 
}


//For now all the lights all point lights
vec4 ComputeLightingValue(int lightIdx)
{
    Material curMat = materialArray.mats[matIdx];
    Light curLight = lightArray.lights[lightIdx];
    
    switch(curLight.type)
    {
    case 0:
        return Phong(curMat, curLight, normalize(curLight.position - fragPos), curLight.type);
    case 1:
        return Phong(curMat, curLight, normalize(-curLight.direction), curLight.type);
    }
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
    {
        float shadow = textureProj(lightFragPos, vec2(0.0f, 0.0f) ,bias);
        outColor = (1.0f - shadow) * vec4(fragColor, 1.0f);
    }
}