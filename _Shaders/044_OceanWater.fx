#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
#include "000_Terrain.fx"

float PI = 3.14159265f;


float4 PS(MeshOutput input) : SV_Target0
{
    return VS_Shadow(input, VS_AllLight(input));
}

///////////////////////////////////////////////////////////////////////////////

float4 PS_Terrain(TerrainOutput input) : SV_Target0
{
    return VS_Shadow_Terrain(input, VS_AllLight_Terrain(input));
}

///////////////////////////////////////////////////////////////////////////////
cbuffer CB_Ocean
{
    float WaveFrequency;
    float WaveAmplitude;

    float2 TextureScale;

    float2 BumpSpeed;
    float BumpHeight;
    float HeightRatio; //높이 보정값

    float4 ShallowColor; //얕은색
    float4 DeepColor; //깊은색

    float FresnelBias; //편향
    float FresnelPower; //강도
    float FresnelAmount; //보정값
    float ShoreBlend; //알파값 변화정도

    float2 OceanSize; //버텍스의 전체크기
};

struct VertexOutput
{
    float4 Position : SV_Position0;
    float3 oPosition : Position1;
    float3 wPosition : Position2;
    float2 Uv : Uv0;

    float3x3 Tangent : Tangent0;
    float2 Bump[3] : Bump0;
};

struct Wave
{
    float Frequency;    //파동
    float Amplitude;    //진폭
    float Phase;        //위상
    float2 Direction;   //방향
};

float EvaluateWave(Wave wave,float2 position)
{
    //float s = 0, c = 0;
    float s = sin(dot(wave.Direction, position) * wave.Frequency + Time) * wave.Phase;

    return wave.Amplitude * s;
}

float EvaluateFFT(Wave wave, float2 position)
{
    float f = 0;
    float s = 0,c=0;
    s -= sin(dot(wave.Direction, position) * wave.Frequency + Time) * wave.Phase;
    c += cos(dot(wave.Direction, position) * wave.Frequency + Time) * wave.Phase;
    return wave.Amplitude * f;
}

VertexOutput VS_Ocean(VertexTexture input)
{
    VertexOutput output;
    Wave wave[3] =
    {
        0.0f, 0.0f, 0.50f, float2(-1.0f, 0.0f),
        0.0f, 0.0f, 1.50f, float2(-0.70f, 0.70f),
        0.0f, 0.0f, 0.25f, float2(0.20f, 1.0f)
    };

    wave[0].Frequency = WaveFrequency;
    wave[0].Amplitude = WaveAmplitude;
    wave[1].Frequency = WaveFrequency*2.0f;
    wave[1].Amplitude = WaveAmplitude*0.5f;
    wave[2].Frequency = WaveFrequency*3.0f;
    wave[2].Amplitude = WaveAmplitude*1.0f;
    
    float ddx = 0, ddy = 0;
    //[unroll(3)]
    for (int i = 0; i < 3;i++)
        input.Position.y = EvaluateWave(wave[i], input.Position.xz);
    //input.Position.y = EvaluateWave(wave[1], input.Position.xz);
    //input.Position.z = EvaluateWave(wave[2], input.Position.xz);

    output.oPosition = input.Position.xyz;
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;
    output.Position = ViewProjection(output.Position);
    //output.Uv = input.Uv;


    return output;
}

float4 PS_Ocean(VertexOutput input) : SV_Target0
{
    return float4(input.oPosition, 0.5f);
}


RasterizerState RS
{
    CullMode = Front;
};
technique11 T0
{
    /* Depth */
    P_RS_VP(P0, RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1, RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2, RS, VS_Depth_Animation, PS_Depth)
   
    /* Render */
    P_VP(P3, VS_Mesh, PS)
    P_VP(P4, VS_Model, PS)
    P_VP(P5, VS_Animation, PS)

    P_VP(P6, VS_Terrain, PS_Terrain)
    P_BS_VP(P7, AlphaBlend, VS_Ocean, PS_Ocean)

}