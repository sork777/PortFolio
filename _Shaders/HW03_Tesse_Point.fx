#include "000_Header.fx"
#include "000_Light.fx"

float4 VS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0); 
}
struct ConstantHullOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

ConstantHullOutput HS_Constant()
{
    ConstantHullOutput output;
   
    float tessFactor = 18.0;
    output.Edges[0] = output.Edges[1] = output.Edges[2] = output.Edges[3] = tessFactor;
    output.Inside[0] = output.Inside[1] = tessFactor;

    return output;
}

struct HullOutput
{
    float4 HemiDir : Position0;
};
static const float3 HemilDir[2] =
{
    float3(1.0, 1.0, 1.0),
	float3(-1.0, 1.0, -1.0)
};
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
HullOutput HS(uint PatchID : SV_PrimitiveID)//uintpointID:SV_OutputControlPointID)
{
    HullOutput output;
    output.HemiDir = float4(HemilDir[PatchID], 1);
    return output;
}

struct DomainOutput
{
    float4 Position : SV_Position0;
    float2 cpPos : TEXCOORD0;

};

[domain("quad")]
DomainOutput DS(ConstantHullOutput input, const OutputPatch<HullOutput, 4> patch, float2 uv : SV_DomainLocation)
{

    float2 posClipSpace =uv * 2.0 - 1.0;

	// Find the absulate maximum distance from the center
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Generate the final position in clip-space
    float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * patch[0].HemiDir.xyz);
    float4 posLS = float4(normDir.xyz, 1.0);
	
    DomainOutput output;
    output.Position = posLS;
    output.cpPos = output.Position.xy / output.Position.w;
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