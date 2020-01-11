#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

uint Death[100];

struct ModelOutput
{
    float4 Position : SV_Position0;
    float3 oPosition : Position1;
    float3 wPosition : Position2;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
    uint ID : Id0;
};

ModelOutput VS_InstModel(VertexModel input)
{
    ModelOutput output;
    MeshOutput temp = VS_Model(input);
    output.Position = temp.Position;
    output.oPosition = temp.oPosition;
    output.wPosition = temp.wPosition;
    output.Uv=temp.Uv;
    output.Normal=temp.Normal;
    output.Tangent = temp.Tangent;
    output.ID = input.InstID;
    return output;
}


float4 PS(ModelOutput input) : SV_Target0
{

    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    
    float3 normal = normalize(input.Normal);
    float NdotL = saturate(dot(normal, -GlobalLight.Direction));
    float4 effect = float4(1, 1, 1, 1);
    [flatten]
    if (Death[input.ID] == 1)
    {
        effect = float4(2, 0.5f, 0.5f, 1);
    }
    return (diffuse * NdotL) * effect;
}

float4 PS2(MeshOutput input) : SV_Target0
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);
    Texture(Material.Specular, SpecularMap, input.Uv);
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputePointLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    return float4(MaterialToColor(result), 1);
}

RasterizerState RS
{
    Fillmode = Wireframe;
};
technique11 T0
{
    P_VP(P0, VS_InstModel, PS)
    P_VP(P1, VS_InstModel, PS2)
    P_RS_VP(P2, RS, VS_InstModel, PS)
    //P_VP(P1, VS_Mesh, PS)

  //  P_RS_VP(P2, RS, VS_Mesh, PS)
}