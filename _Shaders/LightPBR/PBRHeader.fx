//PBR : Physically Based Rendering
//- Global Illumination -> IBL(Image Based Lighting)
//- Energy Conservation
//- Reflectivity -> Diffuse & Specular
//- Microsurface -> Roughness
//- Fresnel's Law -> BRDF
//- Metalicity

//BRDF : Bidirectional Reflectance Distribution Function
//- 반사율 분포를 출력하는 함수여야함
//- 양방향성을 가져야함
//- 광원과 관찰자를 매개변수로 받야함
//- 함수여야함 -> 같은 매개변수에서는 항상 같은 값이 나와야함

//Diffuse BRDF
//Specular BRDF

//#include "../000_Header.fx"
//#include "../000_Light.fx"
struct MaterialPBRDesc
{
    float4 Albedo;
    float4 Emissive;
    float3 F0;
    float Loughness;    
};

cbuffer CB_MaterialPBR
{
    MaterialPBRDesc MaterialPBR;
};
Texture2D AlbedoMap;
Texture2D LoughnessMap;
Texture2D MetalicMap;
Texture2D BRDFLUT;

SamplerState BasicSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 16;
    ComparisonFunc = Always;
};

SamplerState AnisotropicSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = Wrap;
    AddressV = Wrap;
};
float NormalDistributionGGXTR(float3 normalVec, float3 halfwayVec, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a; // a2 = a^2
    float NdotH = max(dot(normalVec, halfwayVec), 0.0); // NdotH = normalVec.halfwayVec
    float NdotH2 = NdotH * NdotH; // NdotH2 = NdotH^2
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return nom / denom;
}


float GeometrySchlickGGX(float NdotV, float roughness)  // k is a remapping of roughness based on direct lighting or IBL lighting
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}


float GeometrySmith(float3 normalVec, float3 viewDir, float3 lightDir, float k)
{
    float NdotV = max(dot(normalVec, viewDir), 0.0f);
    float NdotL = max(dot(normalVec, lightDir), 0.0f);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)   // cosTheta is n.v and F0 is the base reflectivity
{
    return (F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0f));
}
float FresnelSchlick(float f0, float fd90, float view)
{
    return f0 + (fd90 - f0) * pow(max(1.0f - view, 0.1f), 5.0f);
}
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)   // cosTheta is n.v and F0 is the base reflectivity
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0f);
}

float Disney(float NdotL, float LdotH, float NdotV, float roughness)
{
   
    float energyBias = lerp(0.0f, 0.5f, roughness);
    float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
    float fd90 = energyBias + 2.0f * (LdotH * LdotH) * roughness;
    float f0 = 1.0f;

    float lightScatter = FresnelSchlick(f0, fd90, NdotL);
    float viewScatter = FresnelSchlick(f0, fd90, NdotV);

    return lightScatter * viewScatter * energyFactor;
}

float4 CalcSkyIrradiance(float3 normal)
{
    
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = cross(up, normal);
    up = cross(normal, right);

    float sampleDelta = 0.25f;
    float nrSamples = 0.0f;

    for (float phi = 0.0f; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = (tangentSample.x * right) + (tangentSample.y * up) + (tangentSample.z * normal);

            irradiance += SkyCubeMap.Sample(BasicSampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1 / nrSamples);

    return float4(irradiance, 1.0f);
}