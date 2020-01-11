#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"


float4 PS(MeshOutput input) : SV_Target0
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);

    NormalMapping(input.Uv, input.Normal, input.Tangent);

    Texture(Material.Specular, SpecularMap, input.Uv);
   
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputePointLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputeSpotLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputeCapsuleLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);
   
    return float4(MaterialToColor(result), 1);
    
    //pixel = ceil(pixel * 5) / 5.0f;
    //float3 normal = normalize(input.Normal);
    //float NdotL = saturate(dot(normal, -GlobalLight.Direction));
    //NdotL = ceil(NdotL * 5) / 5.0f;
    //
    //return float4(NdotL*pixel.rgb, 1);
    //return CapsuleLights[0].Diffuse;
}


float4 PS2(MeshOutput input) : SV_Target0
{
    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    
    float3 normal = normalize(input.Normal);
    float NdotL = saturate(dot(normal, -GlobalLight.Direction));
    return (diffuse * NdotL);
}
float4 PS3(MeshOutput input) : SV_Target0
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);

    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);
    return float4(MaterialToColor(result), 1);
}


RasterizerState RS
{
    Fillmode = Wireframe;
};
technique11 T0
{
    P_VP(P0, VS_Mesh,PS)
    P_VP(P1, VS_Model, PS)

    P_RS_VP(P2, RS, VS_Mesh, PS)
    P_RS_VP(P3, RS, VS_Model, PS)
    
    P_VP(P4, VS_Mesh, PS2)
    P_VP(P5, VS_Mesh, PS3)
}