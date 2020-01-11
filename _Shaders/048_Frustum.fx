#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

float4 Planes[6];
struct VertexOutput
{
    //sv는 픽셀 쉐이더에서 직접 접근 불가
    float4 Position : SV_Position0;
    float4 wvpPosition : Position1;
    float3 oPosition : Position2;
    float3 wPosition : Position3;
    float4 sPosition : Position4;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;

    //클립 컬 합쳐서 2개 까지만 가능
    float4 Cull : SV_CullDistance0;
    float4 Cull2 : SV_CullDistance1;
};

VertexOutput VS_Cull(VertexModel input)
{
    VertexOutput output;

    SetModelWorld(World, input);

    output.oPosition = input.Position.xyz;
    
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = ViewProjection(output.Position);
    output.wvpPosition = output.Position;
   
    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Uv = input.Uv;

    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.Cull.x = dot(float4(output.wPosition, 1), Planes[0]);
    output.Cull.y = dot(float4(output.wPosition, 1), Planes[1]);
    output.Cull.z = dot(float4(output.wPosition, 1), Planes[2]);
    output.Cull.w = 0.0f;
    output.Cull2.x = dot(float4(output.wPosition, 1), Planes[3]);
    output.Cull2.y = dot(float4(output.wPosition, 1), Planes[4]);
    output.Cull2.z = dot(float4(output.wPosition, 1), Planes[5]);
    output.Cull2.w = 0.0f;
    return output;

}

float4 PS(MeshOutput input) : SV_Target0
{
//    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    
//    float3 normal = normalize(input.Normal);
//    float NdotL = saturate(dot(normal, -GlobalLight.Direction)); 
//    return (diffuse * NdotL);
//}


//float4 PS2(MeshOutput input) : SV_Target0
//{
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
    //return CapsuleLights[0].Diffuse;
    
}

float4 PS_Cull(VertexOutput input) : SV_Target0
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
    //return CapsuleLights[0].Diffuse;
    
}
technique11 T0
{
    P_VP(P0, VS_Mesh,PS)
    P_VP(P1, VS_Cull, PS_Cull)
}