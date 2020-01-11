#include "000_Header.fx"
#include "000_Light.fx"

uint Factor[6];
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
    /* 우리가 넘겨줄 자를 수치? */
    float Edge[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

//1번만 콜? 얼마만큼 분할 할지 가져옴?
//                                                     ↓ controll_point
ConstantHullOutput HS_Constant(InputPatch<VertexOutput,4> input)
{
    ConstantHullOutput output;
    output.Edge[0] = Factor[0];
    output.Edge[1] = Factor[1];
    output.Edge[2] = Factor[2];
    output.Edge[3] = Factor[3];

    output.inside[0] = Factor[4];
    output.inside[1] = Factor[5];

    return output;
}

struct HullOutput
{
    float4 Position : Position0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
//controlpoint 만큼 콜?
//자르기 전에 행동?
//여기서 정해주면 시스템에서 알아서 잘라줌? TS에서?
HullOutput HS(InputPatch<VertexOutput, 4> input,uint pointID:SV_OutputControlPointID)
{
    HullOutput output;
    output.Position = input[pointID].Position;
    return output;
}

struct DomainOutput
{
    float4 Position : SV_Position0;
};

[domain("quad")]                                                                 //사각형 이상은 무조건 얘
DomainOutput DS(ConstantHullOutput input, const OutputPatch<HullOutput, 4> patch, float2 uv : SV_DomainLocation)
{
    DomainOutput output;
    //uv.y는 이미지랑 뒤집혀 있음
    float3 v1 = lerp( patch[0].Position.xyz , patch[1].Position.xyz,1-uv.y);
    float3 v2 = lerp( patch[2].Position.xyz , patch[3].Position.xyz,1-uv.y);
    float3 position = lerp(v1, v2, uv.x);
    output.Position = float4(position, 1);  //원래 여기서 wvp 변환 들어감

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