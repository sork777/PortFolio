#include "000_Header.fx"
#include "000_Light.fx"

cbuffer CB_Render2D
{
    matrix View2D;
    matrix Projection2D;
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
cbuffer CB_Values
{
    float Noise;
    int Seed;
};

float4 PS_Shake(VertexOutput input) : SV_TARGET0
{
    float2 uv = input.Uv;
    float noise = Seed * Time;
    float2 distort = 0;
    distort.x = fmod(noise, Noise);
    distort.y = fmod(noise, Noise+ 0.002f);

    float4 pixel = DiffuseMap.Sample(LinearSampler, uv+distort);
    
    return float4(pixel.rgb, 1);
}

float4 PS_Noise(VertexOutput input) : SV_TARGET0
{
    float2 uv = input.Uv;
    float noise = Seed * Time * sin(uv.x + uv.y);
    
    float2 distort = 0;
    distort.x = fmod(noise, Noise);
    distort.y = fmod(noise, Noise + 0.002f);

    float4 pixel = DiffuseMap.Sample(LinearSampler, uv + distort);
    
    return float4(pixel.rgb, 1);
}


float4 PS_Distort(VertexOutput input) : SV_TARGET0
{
    float2 uv = input.Uv;
    float noise = Seed * Time * sin(uv.x + uv.y+Time);
    
    float2 distort = 0;
    distort.x = fmod(noise, Noise);
    distort.y = fmod(noise, Noise + 0.002f);

    float4 pixel = DiffuseMap.Sample(LinearSampler, uv + distort);
    
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
    P_DSS_VP(P0, Depth, VS, PS_Shake)
    P_DSS_VP(P1, Depth, VS, PS_Noise)
    P_DSS_VP(P2, Depth, VS, PS_Distort)
}