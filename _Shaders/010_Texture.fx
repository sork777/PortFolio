#include "000_Header.fx"

float2 Uv;

struct VertexOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

VertexOutput VS(VertexTexture input)
{
    VertexOutput output;
    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);
    
    output.Uv = input.Uv;
    return output;
}

float4 PS(VertexOutput input) : SV_Target0
{
    [flatten]
    if(input.Uv.x<Uv.x)
        return float4(0, 0, 0, 1);


    return DiffuseMap.Sample(Sampler, input.Uv);
}

///////////////////////////////////////////////////////////////////////////////


SamplerState Address_Wrap
{
    //이 명칭은 SamplerState의 변수 명임
    AddressU = WRAP; //가로로 늘어나는 것
    AddressV = WRAP; //세로로 늘어나는 것
};

SamplerState Address_Mirror
{
    AddressU = MIRROR; //가로로 늘어나는 것
    AddressV = MIRROR; //세로로 늘어나는 것
};

SamplerState Address_Clamp
{
    AddressU = CLAMP; //가로로 늘어나는 것
    AddressV = CLAMP; //세로로 늘어나는 것
};

SamplerState Address_Border
{
    AddressU = BORDER; //가로로 늘어나는 것
    AddressV = BORDER; //세로로 늘어나는 것

    BorderColor = float4(0, 0, 1, 1);
};

uint Address;
float4 PS_Address(VertexOutput input) : SV_Target0
{
    //PS에서 switch문 쓸때
    [branch]
    switch (Address)
    {
        case 0:
            return DiffuseMap.Sample(Address_Wrap, input.Uv);
        case 1:
            return DiffuseMap.Sample(Address_Mirror, input.Uv);
        case 2:
            return DiffuseMap.Sample(Address_Clamp, input.Uv);
        case 3:
            return DiffuseMap.Sample(Address_Border, input.Uv);
    }

    return float4(0, 0, 0, 1);
}

///////////////////////////////////////////////////////////////////////////////

SamplerState Point
{
    Filter = MIN_MAG_MIP_POINT;
};

SamplerState Linear
{
    Filter = MIN_MAG_MIP_LINEAR;
};

uint Filter;
float4 PS_Filter(VertexOutput input) : SV_Target0
{
    //PS에서 switch문 쓸때
    [branch]
    switch (Filter)
    {
        case 0:
            return DiffuseMap.Sample(Point, input.Uv);
        case 1:
            return DiffuseMap.Sample(Linear, input.Uv);    
    }

    return float4(0, 0, 0, 1);
}
///////////////////////////////////////////////////////////////////////////////
technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

    pass P1
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Address()));
    }

    pass P2
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Filter()));
    }
}
