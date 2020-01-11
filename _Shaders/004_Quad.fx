#include "000_Header.fx"

float4 Color;


struct VertexInput
{
	float4 Position : Position0;
};

struct VertexOutput
{
	float4 Position : SV_Position0;
};

VertexOutput VS(VertexInput input)
{
    //WVP ��ȯ�� VS����
	VertexOutput output;
    //mul : ��İ����� ���Ͱ�
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
	return output;
}

float4 PS(VertexOutput input) : SV_Target0
{
	return Color;
}

technique11 T0
{
	pass P0
	{
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

    pass P1
    {        
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}