#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Terrain.fx"


struct VertexOutput
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float3 Normal : Normal0;
    float2 Uv : Uv0;
    float3 Color : Color0;
    float4 Alpha : Alpha0;
   
};

VertexOutput VS(VertexTextureColorNormal input)
{
    //WVP 변환은 VS에서
    VertexOutput output;
    //mul : 행렬곱이자 벡터곱
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position; //swizzling
    output.Position = ViewProjection(output.Position);
   
    output.Alpha = input.Color;
    output.Normal = WorldNormal(input.Normal);
    output.Uv = input.Uv;
    
    return output;
}

float4 PS(VertexOutput input) : SV_Target0
{
    float2 uv = input.Uv;
    float4 color = input.Alpha;
    float4 base = BaseMap.Sample(LinearSampler, uv);
    float4 layer;
    float alpha;
    float4 diffuse;

    layer = LayerMap0.Sample(LinearSampler, uv);
    alpha = color.r;
    diffuse = lerp(base, layer, alpha);

    layer = LayerMap1.Sample(LinearSampler, uv);
    alpha = color.g;
    diffuse = lerp(diffuse, layer, alpha);

    layer = LayerMap2.Sample(LinearSampler, uv);
    alpha = color.b;
    diffuse = lerp(diffuse, layer, alpha);

    float3 normal = normalize(input.Normal);
    float NdotL = saturate(dot(normal, -GlobalLight.Direction)); 
	
	float3 brushColor = GetBrushColor(input.wPosition); //rasterizer에서 보간이 일어나서 깔금하게나온다.

    float3 gridColor = GetLineColor(input.wPosition); //rasterizer에서 보간이 일어나서 깔금하게나온다.
    return (diffuse * NdotL) + float4(gridColor, 1) + float4(brushColor, 1);

    //return float4(gridColor, 1);
}

RasterizerState RS
{
	Fillmode = Wireframe;
};

technique11 T0
{
	P_VP(P0, VS, PS)
	P_RS_VP(P1, RS, VS, PS)
}