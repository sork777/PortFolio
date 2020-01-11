#include "000_Header.fx"
#include "000_Light.fx"

cbuffer CB_Render2D
{
    matrix View2D;
    matrix Projection2D;
};
cbuffer CB_Values
{
    float2 MapSize;
    float Sharpening;
};

//-----------------------------------------------------------------------------
// Pass0
//-----------------------------------------------------------------------------
struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float2 Uv : Uv0;
    float2 Uv1 : Uv1;
    float2 Uv2 : Uv2;
    float2 Uv3 : Uv3;
    float2 Uv4 : Uv4;
};

VertexOutput VS(VertexTexture input)
{
    
    VertexOutput output;

    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, View2D);
    output.Position = mul(output.Position, Projection2D);
    

    float2 offset = 1 / MapSize;

    output.Uv = input.Uv;
    output.Uv1 = input.Uv + float2(0, -offset.y);
    output.Uv2 = input.Uv + float2(-offset.x, 0);
    output.Uv3 = input.Uv + float2(+offset.x, 0);
    output.Uv4 = input.Uv + float2(0, +offset.y);

    return output;
}


float4 PS_Sharpen(VertexOutput input) : SV_TARGET0
{
    float4 center = DiffuseMap.Sample(LinearSampler, input.Uv);
    float4 top = DiffuseMap.Sample(LinearSampler, input.Uv1);
    float4 left = DiffuseMap.Sample(LinearSampler, input.Uv2);
    float4 right = DiffuseMap.Sample(LinearSampler, input.Uv3);
    float4 bottom = DiffuseMap.Sample(LinearSampler, input.Uv4);

    float4 color = 4 * center - (top + bottom + left + right);
    return color + Sharpening*center;

}


float4 PS_ToonTest(VertexOutput input) : SV_TARGET0
{
    float3 viewpoint = ViewPosition();
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    float4 sharpen = pixel - PS_Sharpen(input);
     
    pixel = ceil(pixel * 5) / 5.0f;    

    [flatten]
    if(sharpen.r<0.3f &&
        sharpen.r < 0.3f &&
        sharpen.r < 0.3f)
        return float4(0, 0, 0, 1);
    return float4(pixel.rgb, 1);

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
    P_DSS_VP(P0, Depth, VS, PS_Sharpen)
    P_DSS_VP(P1, Depth, VS, PS_ToonTest)
}