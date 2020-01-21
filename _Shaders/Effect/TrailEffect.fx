#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"

#define MAX_TRAILBUFFER_COUNT 500

struct TrailDesc
{
    matrix TrailMatrix[MAX_TRAILBUFFER_COUNT];

    int TrailCount;
    float3 VS_TrailMatrix_Padding;
};
cbuffer CB_Trail
{
    TrailDesc Trail;
};
Texture2D TrailTexture;
Texture2D MaskTexture;


struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
};

PixelInput VS_Trail(VertexTexture input)
{
    PixelInput output;

    //float rate = 1.0f / (float) Trail.TrailCount;
    int idx = (int) ((input.Uv.x) * Trail.TrailCount);
    idx = min(idx, Trail.TrailCount - 1);

    matrix mat = Trail.TrailMatrix[idx];

    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, mat);
    output.Position = ViewProjection(output.Position);

    output.Uv = input.Uv;

    return output;
}

float4 PS_Trail(PixelInput input) : SV_Target0
{
    input.Uv.y = 1.0f - input.Uv.y;
    float4 color = TrailTexture.Sample(LinearSampler, input.Uv);
    float4 Mask = MaskTexture.Sample(LinearSampler, input.Uv);

    return color*Mask;
    //return float4(1, 0, 0, 1);
}
technique11 T0
{
	P_RS_BS_VP(P0,NoneRS,TrailBlend, VS_Trail, PS_Trail)
	//P_RS_VP(P0, TrailRS, VS_Trail, PS_Trail)
}