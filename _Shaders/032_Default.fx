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

float4 PS_Inverse(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    
    return float4(1 - pixel.rgb, 1);
}

float4 PS_Grayscale(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    float color = (pixel.r + pixel.g + pixel.b) / 3.0f;
    return float4(color, color, color,1.0f);
}
float4 PS_Grayscale2(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    float color = dot(pixel.rgb, float3(0.299f, 0.587f, 0.114f));
    return float4(color, color, color, 1.0f);
}

float4 Tone;
float4 PS_Tone(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    pixel.r *= Tone.r;
    pixel.g *= Tone.g;
    pixel.b *= Tone.b;

    return pixel;
}

float4 Gamma;
float4 PS_Gamma(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    pixel.r = pow(pixel.r, 1 / Gamma.r);
    pixel.g = pow(pixel.g, 1 / Gamma.g);
    pixel.b = pow(pixel.b, 1 / Gamma.b);

    return pixel;
}


uint Bits;
float4 PS_BitSlice(VertexOutput input) : SV_TARGET0
{
    float4 pixel = DiffuseMap.Sample(LinearSampler, input.Uv);
    int4 pixelint =   pixel *255;
    
    pixelint.r >>= Bits;
    pixelint.g >>= Bits;
    pixelint.b >>= Bits;
    
    pixelint.r <<= Bits;
    pixelint.g <<= Bits;
    pixelint.b <<= Bits;
    pixel = pixelint;
    pixel /= 255;

    return float4(pixel.rgb, 1.0f);
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
    P_DSS_VP(P0, Depth, VS, PS_Inverse)
    P_DSS_VP(P1, Depth, VS, PS_Grayscale)
    P_DSS_VP(P2, Depth, VS, PS_Grayscale2)
    P_DSS_VP(P3, Depth, VS, PS_Tone)
    P_DSS_VP(P4, Depth, VS, PS_Gamma)
    P_DSS_VP(P5, Depth, VS, PS_BitSlice)
}