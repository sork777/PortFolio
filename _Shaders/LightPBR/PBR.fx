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

#include "../000_header.fx"
struct MaterialPBRDesc
{
    float4 Albedo;
    float3 F0;
    float Loughness;
    float3 Emissive;
    float Metalic;
};

cbuffer CB_MaterialPBR
{
    MaterialPBRDesc MaterialPBR;
};
