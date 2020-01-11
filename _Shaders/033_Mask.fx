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

float3 Size;

float4 PS_Embossing(VertexOutput input) : SV_TARGET0
{
    float Mask[3][3] =
    {
        { 2, 1, 0 },
        { 1, 0, -1 },
        { 0, -1, -2 }
    };
    float2 uv;
    float4 pixel = 0;
    for (int i = 0; i < 3;i++)
    {
        for (int j = 0; j < 3; j++)
        {
            uv = input.Uv - float2((i - 1) / Size.x, (j - 1) / Size.y);
            pixel += DiffuseMap.Sample(LinearSampler, uv) * Mask[i][j];

        }

    }
    
    pixel = dot(pixel.rgb, float3(0.299f, 0.587f, 0.114f));
    return float4(pixel.rgb, 1);
}

float4 PS_Blurring(VertexOutput input) : SV_TARGET0
{
    float2 uv = input.Uv;
    float4 pixel = float4(0, 0, 0, 0);

    for (int i = -2; i < 3;i++)
    {
        for (int j = -2; j < 3; j++)
        {
            pixel += DiffuseMap.Sample(LinearSampler, float2(uv.x - i/Size.x, uv.y - j/Size.y));
        }
    }
    pixel /= 25;
    return float4(pixel.rgb, 1);
}


float4 PS_Shapen(VertexOutput input) : SV_TARGET0
{
    float Mask[3][3] =
    {
        { 1, -2,1 },
        { -2, 5, -2 },
        { 1, -2, 1 }
    };
    float2 uv;
    float4 pixel = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            uv = input.Uv - float2((i - 1) / Size.x, (j - 1) / Size.y);
            pixel += DiffuseMap.Sample(LinearSampler, uv) * Mask[i][j];

        }

    }
    
    return float4(pixel.rgb, 1);

}


float4 PS_ToonTest(VertexOutput input) : SV_TARGET0
{
   
    float4 pixel = PS_Shapen(input);

    [flatten]
    if (pixel.r < 0.3f &&
        pixel.g < 0.3f &&
        pixel.b < 0.3f)
        return float4(0, 0, 0, 1);
   
    return DiffuseMap.Sample(LinearSampler, input.Uv);

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
    P_DSS_VP(P0, Depth, VS, PS_Embossing)
    P_DSS_VP(P1, Depth, VS, PS_Blurring)
    P_DSS_VP(P2, Depth, VS, PS_Shapen)
    P_DSS_VP(P3, Depth, VS, PS_ToonTest)   
}