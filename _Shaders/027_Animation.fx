#include "000_Header.fx"
#include "000_Light.fx"
#include "LightPBR/PBR.fx"
#include "000_Model.fx"

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

    return float4(MaterialToColor(result), 1);
    //return float4(diffuse * NdotL, 1);
}
float4 PS_PBRTest(MeshOutput input) : SV_Target0
{
    return PBRmain(input);
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
}
