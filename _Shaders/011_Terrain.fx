#include "000_Header.fx"
#include "000_Light.fx"

Texture2D BaseMap;

struct VertexOutput
{
	float4 Position : SV_Position0;
    float3 Normal : Normal0;
    float2 Uv : Uv0;

};

VertexOutput VS(VertexTextureNormal input)
{
	VertexOutput output;
    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);

    //Normal이 float3이라
    output.Normal = WorldNormal(input.Normal);
    output.Uv = input.Uv;

	return output;
}


float4 PS(VertexOutput input) : SV_Target0
{
    float4 diffuse = BaseMap.Sample(LinearSampler, input.Uv);

    float3 normal = normalize(input.Normal);
    float NdotL = saturate(dot(normal, -GlobalLight.Direction));
    //
    return diffuse * NdotL;
    //테스트용
    //return float4(LightDirection*0.5f+0.5f,1);
}



technique11 T0
{
	pass P0
	{
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

    pass P1
    {
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

}
