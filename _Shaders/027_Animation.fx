#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
/*
    디퍼드 관련 파이프라인 설정 필요.0902
*/
float4 PS(MeshOutput input) : SV_Target0
{
    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    Material.Diffuse *= diffuse;
    
    NormalMapping(input.Uv, input.Normal, input.Tangent);
    float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);
    Material.Diffuse *= NdotL; 
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    float3 color = MaterialToColor(result);
    //color = VS_Shadow(input, float4(color, 1));
    return float4(color, 1);
}

float4 PS_PBRTest(MeshOutput input) : SV_Target0
{
    float4 color = CalcNormaltoPBR(input);
    //color=VS_Shadow(input,color);
    return color;
}

float4 WirePS(MeshOutput input) : SV_Target0
{
    return float4(0, 0, 0, 1);
}

technique11 T0
{
    P_VP(P0, VS_Mesh, PS)
    P_VP(P1, VS_Model, PS)
    P_VP(P2, VS_Animation, PS)
}

technique11 T_PBR
{
    P_VP(P0, VS_Mesh, PS_PBRTest)
    P_VP(P1, VS_Model, PS_PBRTest)
    P_VP(P2, VS_Animation, PS_PBRTest)
}
