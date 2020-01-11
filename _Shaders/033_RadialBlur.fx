#include "000_Header.fx"
#include "000_Light.fx"

#define MAX_RADIAL_BLUR_COUNT 32

Texture2D BlurMap0;
Texture2D BlurMap1;
Texture2D BlurMap2;
Texture2D BlurMap3;
Texture2D BlurMap4;
Texture2D BlurMap5;
Texture2D BlurMap6;
Texture2D BlurMap7;

cbuffer CB_Render2D
{
    matrix View2D;
    matrix Projection2D;
};
cbuffer CB_Values
{
    float2 MapSize;
    
    uint BlurCount;
    float Radius;
    float Amount;
    float Offset;

};

//-----------------------------------------------------------------------------
// Pass0
//-----------------------------------------------------------------------------
struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float2 Uv : Uv0;
};

VertexOutput VS(VertexTexture input)
{
    VertexOutput output;

    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, View2D);
    output.Position = mul(output.Position, Projection2D);
    output.Uv = input.Uv;

    return output;
}

float4 PS(VertexOutput input) : SV_TARGET0
{
    /* 원의 중심... */
    float2 radius = input.Uv - float2(0.5f, 0.5f);
    float r = length(radius) + Offset;
    radius /= r;
    r = 2 * r / Radius;
    r = saturate(r);
    
    float2 delta = radius * r * r * Amount / BlurCount;
    delta = -delta;
    float4 color = 0;

    //[unroll(MAX_RADIAL_BLUR_COUNT)]
    for (int i = 0; i < BlurCount;i++)
    {
        color += DiffuseMap.Sample(LinearSampler, input.Uv);
        input.Uv += delta;
    }

    color /= BlurCount;
    return float4(color.rgb, 1);
}

float4 PS_Motion(VertexOutput input) : SV_TARGET0
{
    float4 color = BlurMap0.Sample(LinearSampler, input.Uv) ;
    color += BlurMap1.Sample(LinearSampler, input.Uv) ;
    color += BlurMap2.Sample(LinearSampler, input.Uv) ;
    color += BlurMap3.Sample(LinearSampler, input.Uv) ;
    color += BlurMap4.Sample(LinearSampler, input.Uv) ;
    color += BlurMap5.Sample(LinearSampler, input.Uv) ;
    color += BlurMap6.Sample(LinearSampler, input.Uv) ;
    color += BlurMap7.Sample(LinearSampler, input.Uv);
    color /= 8.0f;
    return float4(color.rgb, 1);
}
//-----------------------------------------------------------------------------
// Techniques
//-----------------------------------------------------------------------------
DepthStencilState Depth
{
    DepthEnable = false;
};

technique11 T0
{
    P_DSS_VP(P0, Depth, VS, PS)
    P_DSS_VP(P1, Depth, VS, PS_Motion)
}