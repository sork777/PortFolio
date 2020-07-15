#include "000_Header.fx"
#include "000_Light.fx"

struct VertexOutput
{
	float4 Position : SV_Position0;
    float4 Color : Color0;
    
    float4 Cull : SV_CullDistance0;
    float4 Cull2 : SV_CullDistance1;
};

VertexOutput VS(VertexColor input)
{
	VertexOutput output;
    output.Position = WorldPosition(input.Position);
    float3 wPosition = output.Position.xyz;
    output.Position = ViewProjection(output.Position);

    output.Color = input.Color;
    
    output.Cull.x = dot(float4(wPosition, 1), Planes[0]);
    output.Cull.y = dot(float4(wPosition, 1), Planes[1]);
    output.Cull.z = dot(float4(wPosition, 1), Planes[2]);
    output.Cull.w = 0.0f;
    output.Cull2.x = dot(float4(wPosition, 1), Planes[3]);
    output.Cull2.y = dot(float4(wPosition, 1), Planes[4]);
    output.Cull2.z = dot(float4(wPosition, 1), Planes[5]);
    output.Cull2.w = 0.0f;
	return output;
}


float4 PS(VertexOutput input) : SV_Target0
{
    return input.Color;
}



technique11 T0
{
    P_VP(P0, VS, PS)
}
