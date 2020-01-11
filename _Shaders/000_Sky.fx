/*static */const float PI = 3.14159265f;
/*static */const float InnerRadius = 6356.7523142f;
/*static */const float OuterRadius = 6356.7523142f * 1.0157313f;

/*static */const float KrESun = 0.0025f * 20.0f; //0.0025f - ���ϸ� ��� * �¾��� ���
/*static */const float KmESun = 0.0010f * 20.0f; //0.0025f - �� ��� * �¾��� ���
/*static */const float Kr4PI = 0.0025f * 4.0f * 3.1415159;
/*static */const float Km4PI = 0.0010f * 4.0f * 3.1415159;

/*static */const float2 RayleighMieScaleHeight = { 0.25f, 0.1f };
/*static */const float Scale = 1.0 / (6356.7523142 * 1.0157313 - 6356.7523142);

/*static */const float g = -0.980f;
/*static */const float g2 = -0.980f * -0.980f;
/*static */const float Exposure = -2.0f;    //���⵵?

/*static */const float ONE = 0.00390625;    //
/*static */const float ONEHALF = 0.001953125;

///////////////////////////////////////////////////////////////////////////////
cbuffer CB_Scatter
{
    float3 WaveLength;
    int SampleCount;

    float3 InvWaveLength;
    float StarIntensity;

    float3 WaveLengthMie;
    float MoonAlpha;
};

struct VertexOutput_Scatter
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

struct PixelOutput_Scatter
{
    float4 R : SV_Target0;
    float4 M : SV_Target1;
};
///////////////////////////////////////////////////////////////////////////////

struct VertexOutput_Dome
{
    float4 Position : SV_Position0;
    float3 oPosition : Position1;
    float2 Uv : Uv0;
};

float GetRayleighPhase(float c)
{
    return 0.75f * (1.0f + c);
}

float GetMiePhase(float c, float c2)
{
    float3 result = 0;
    result.x = 1.5f * ((1.0f - g2) / (2.0f + g2));
    result.y = 1.0f + g2;
    result.z = 2.0f * g;

    return result.x * (1.0f + c2) / pow(result.y - result.z * c, 1.5f);
}

float3 HDR(float3 LDR)
{
    return 1.0f - exp(Exposure * LDR);
}

///////////////////////////////////////////////////////////////////////////////
struct VertexOutput_Moon
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

///////////////////////////////////////////////////////////////////////////////

cbuffer CB_Cloud
{
    float CloudTiles;
    float CloudCover = -0.1f;
    float CloudSharpness = 0.25f;
    float CloudSpeed = 0.01f;
};

struct VertexOutput_Cloud
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
    float2 oUv : Uv1;
};

float Fade(float t)
{
  // return t * t * (3.0 - 2.0 * t);
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

Texture2D CloudMap;
float Noise(float2 P)
{
    float2 Pi = ONE * floor(P) + ONEHALF;
    float2 Pf = frac(P);

    float2 grad00 = CloudMap.Sample(LinearSampler, Pi).rg * 4.0 - 1.0;
    float n00 = dot(grad00, Pf);

    float2 grad10 = CloudMap.Sample(LinearSampler, Pi + float2(ONE, 0.0)).rg * 4.0 - 1.0;
    float n10 = dot(grad10, Pf - float2(1.0, 0.0));

    float2 grad01 = CloudMap.Sample(LinearSampler, Pi + float2(0.0, ONE)).rg * 4.0 - 1.0;
    float n01 = dot(grad01, Pf - float2(0.0, 1.0));

    float2 grad11 = CloudMap.Sample(LinearSampler, Pi + float2(ONE, ONE)).rg * 4.0 - 1.0;
    float n11 = dot(grad11, Pf - float2(1.0, 1.0));

    float2 n_x = lerp(float2(n00, n01), float2(n10, n11), Fade(Pf.x));

    float n_xy = lerp(n_x.x, n_x.y, Fade(Pf.y));

    return n_xy;
}
