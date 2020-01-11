#include "000_Header.fx"
#include "000_Light.fx"
struct InstDesc
{
    Matrix Transform;
    float4 color;
    uint index;
};
struct VertexIntput
{
    float4 Position : Position0;
    float2 Uv : Uv0;

    /* Instancing �� */
    //Inst�� ����
    InstDesc instDesc : Inst0;
    uint InstID : SV_InstanceID0;
};


struct VertexOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
    float4 Color : Color0;

    uint ID : Id0;
};
VertexOutput VS(VertexIntput input)
{
    VertexOutput output;
    /* Instancing �� */
    //input.Position.x += input.InstID;
    output.Position = WorldPosition(input.Position);
    /* Instancing �� */
    output.Position = mul(input.Position, input.instDesc.Transform);
   

    output.Position = ViewProjection(output.Position);
    output.Uv = input.Uv;
    output.ID = input.InstID;
    output.Color = input.instDesc.color;
    return output;
}
Texture2DArray Maps;

float4 PS(VertexOutput input) : SV_Target0
{
    //w�� ��� �ؽ��ĸ� ����Ұ��ΰ�.
    //��ȣ�� ����� ������ �ְ� �ݺ������� ����
    float3 uvw = float3(input.Uv, input.ID%6);
    return DiffuseMap.Sample(LinearSampler, input.Uv);
    //return input.Color;
}

technique11 T0
{
   P_VP(P0, VS, PS)
}