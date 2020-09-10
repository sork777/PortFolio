#include "000_Header.fx"
#include "000_Light.fx"

struct VertexOutput
{
	float4 Position : SV_Position0;
    float4 Color : Color0;
};

VertexOutput VS(VertexColor input)
{
	VertexOutput output;
    output.Position = WorldPosition(input.Position);
    float3 wPosition = output.Position.xyz;
    output.Position = ViewProjection(output.Position);

    output.Color = input.Color;
	return output;
}

float4 PS(VertexOutput input) : SV_Target0
{
    return input.Color;
}

struct PS_Output
{
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Spec : SV_Target2;
    float4 Depth : SV_Target3;
    float4 Emissive : SV_Target4;
};

PS_Output PS_Seperate(VertexOutput input) : SV_Target0
{
    PS_Output output;
    output.Color = float4(input.Color.rgb, 1);
    output.Normal = 0;
    output.Spec = 0;
    output.Depth = 0;
    output.Emissive = 0;
    return output;
}



technique11 T0
{
    P_VP(P0, VS, PS)
    P_VP(P1, VS, PS_Seperate)
}
