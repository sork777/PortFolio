#include "000_Header.fx"
#include "000_Light.fx"

uint Factor[4];
struct VertexOutput
{
    float4 Position : Position0;
};

VertexOutput VS(Vertex input)
{
    VertexOutput output;
    output.Position = input.Position;

    return output;
}

struct ConstantHullOutput
{
    /* �츮�� �Ѱ��� �ڸ� ��ġ? */
    float Edge[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

//1���� ��? �󸶸�ŭ ���� ���� ������?
//                                                     �� controll_point
ConstantHullOutput HS_Constant(InputPatch<VertexOutput,3> input)
{
    ConstantHullOutput output;
    output.Edge[0] = Factor[0];
    output.Edge[1] = Factor[1];
    output.Edge[2] = Factor[2];
    output.inside = Factor[3];

    return output;
}

struct HullOutput
{
    float4 Position : Position0;
};

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HS_Constant")]
//controlpoint ��ŭ ��?
//�ڸ��� ���� �ൿ?
//���⼭ �����ָ� �ý��ۿ��� �˾Ƽ� �߶���? TS����?
HullOutput HS(InputPatch<VertexOutput, 3> input,uint pointID:SV_OutputControlPointID)
{
    HullOutput output;
    output.Position = input[pointID].Position;
    return output;
}

struct DomainOutput
{
    float4 Position : SV_Position0;
};

[domain("tri")]
DomainOutput DS(ConstantHullOutput input, const OutputPatch<HullOutput, 3> patch, float3 uvw : SV_DomainLocation)
{
    DomainOutput output;
    float3 position = uvw.x * patch[0].Position.xyz + uvw.y * patch[1].Position.xyz    +uvw.z * patch[2].Position.xyz;
    output.Position = float4(position, 1);  //���� ���⼭ wvp ��ȯ ��
    return output;
}

float4 PS(DomainOutput input):SV_Target0
{
    return float4(1, 0, 0, 1);
}
RasterizerState RS
{
    Fillmode = Wireframe;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}