//PBR : Physically Based Rendering
//- Global Illumination -> IBL(Image Based Lighting)
//- Energy Conservation
//- Reflectivity -> Diffuse & Specular
//- Microsurface -> Roughness
//- Fresnel's Law -> BRDF
//- Metalicity

//BRDF : Bidirectional Reflectance Distribution Function
//- �ݻ��� ������ ����ϴ� �Լ�������
//- ����⼺�� ��������
//- ������ �����ڸ� �Ű������� �޾���
//- �Լ������� -> ���� �Ű����������� �׻� ���� ���� ���;���

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
