#include "000_Header.fx"


float3 Direction = float3(-1, -1, -1);

struct VertexInput
{
	float4 Position : Position0;    //float4는 포지션만 예외
    float3 Normal : Normal0;
};

struct VertexOutput
{
	float4 Position : SV_Position0;
    float3 Normal : Normal0;
};

VertexOutput VS(VertexInput input)
{
    //WVP 변환은 VS에서
	VertexOutput output;
    //mul : 행렬곱이자 벡터곱
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    //Normal이 float3이라
    output.Normal = mul(input.Normal, (float3x3) World);
    output.Normal = mul(output.Normal, (float3x3) View);
    output.Normal = mul(output.Normal, (float3x3) Projection);

	return output;
}

SamplerState Sampler;

float4 PS(VertexOutput input) : SV_Target0
{
    float4 diffuse = float4(1, 1, 1, 1);

    float3 normal = normalize(input.Normal);
    //빛이 나가는 방향?    램버트 조명식,디퓨즈이미지?
    //clamp 0,1 이랑 같은 효과.
    float NdotL = saturate(dot(normal, -Direction));
    return diffuse * NdotL;
}

technique11 T0
{
	pass P0
	{
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
