#include "000_Header.fx"

#define PI_2 2 * 3.14159265f
struct RaiseDesc
{
    float4 Box;

    float2 Res;
    float2 Position;
    
    float Radius;
    float Rate;
    float Factor;
    int RaiseType;
       
    int SlopDir;
    int SlopRev;
    int SplattingLayer;
    int BrushType;
};

cbuffer CB_Raise
{
    RaiseDesc HRaise;
};
Texture2D<float4> AlphaMap;
RWTexture2D<float4> OutputMap;

Texture2D<float4> AlphaMap2;
RWTexture2D<float4> OutputMap2;
Texture2D PerlinMap;

[numthreads(1024,1,1)]
void QuadRaiseCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    if (CurPixel.x >= HRaise.Box.x
        && CurPixel.x < HRaise.Box.y
        && CurPixel.y < HRaise.Box.w
        && CurPixel.y >= HRaise.Box.z)
    {
        [branch]
        if (HRaise.RaiseType == 0)
            OutputMap[CurPixel.xy] += float4(0, 0, 0, 1.0f / 255.0f) * HRaise.Rate;
        else if (HRaise.RaiseType == 1)
            OutputMap[CurPixel.xy] -= float4(0, 0, 0, 1.0f / 255.0f) * HRaise.Rate;
        else 
            OutputMap[CurPixel.xy] *= float4(1, 1, 1, 0);
    }
}

[numthreads(1024, 1, 1)]
void CircleRaiseCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    float len = length(CurPixel.xy - HRaise.Position);

    if (len<HRaise.Radius)
    {
        float angle = acos(len / HRaise.Radius);
        float factor = sin(angle) * HRaise.Factor * HRaise.Rate;
        [branch]
        if (HRaise.RaiseType == 0)
            OutputMap[CurPixel.xy] += float4(0, 0, 0, 1.0f / 255.0f) * factor;
        else if (HRaise.RaiseType == 1)
            OutputMap[CurPixel.xy] -= float4(0, 0, 0, 1.0f / 255.0f) * HRaise.Rate;
        else 
            OutputMap[CurPixel.xy] *= float4(1, 1, 1, 0);

    }
}

[numthreads(1024, 1, 1)]
void SlopeRaiseCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    if (CurPixel.x >= HRaise.Box.x
        && CurPixel.x < HRaise.Box.y
        && CurPixel.y < HRaise.Box.w
        && CurPixel.y >= HRaise.Box.z)
    {
        float height;
        int slopDir = HRaise.SlopDir;
        int slopRev = HRaise.SlopRev;

        float x = slopRev == 1 ? HRaise.Box.y : HRaise.Box.x;
        float y = slopRev == 1 ? HRaise.Box.w : HRaise.Box.z;

        [branch]
        if (slopDir == 0)
			height = abs(CurPixel.y - y);
        else
			height = abs(CurPixel.x - x);

        height *= tan(HRaise.Factor) * 1.0f / 255.0f;
        
        OutputMap[CurPixel.xy] *= float4(1, 1, 1, 0);
        OutputMap[CurPixel.xy] += float4(0, 0, 0, height);
    }

}

///////////////////////////////////////////////////////////////////////////////

[numthreads(1024, 1, 1)]
void QuadSplattingCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    if (CurPixel.x >= HRaise.Box.x
        && CurPixel.x < HRaise.Box.y
        && CurPixel.y < HRaise.Box.w
        && CurPixel.y >= HRaise.Box.z)
    {
        [branch]
        if (HRaise.SplattingLayer == 0)
            OutputMap[CurPixel.xy] += float4(1.0f / 255.0f, 0, 0, 0) * HRaise.Factor;
        else if (HRaise.SplattingLayer == 1)
            OutputMap[CurPixel.xy] += float4(0, 1.0f / 255.0f, 0, 0) * HRaise.Factor;
        else if (HRaise.SplattingLayer == 2)
            OutputMap[CurPixel.xy] += float4(0, 0, 1.0f / 255.0f, 0) * HRaise.Factor;
    }
}

[numthreads(1024, 1, 1)]
void CircleSplattingCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    float len = length(CurPixel.xy - HRaise.Position);

    if (len < HRaise.Radius)
    {
        float angle = acos(len / HRaise.Radius);
        float factor = sin(angle) * HRaise.Factor;
      [branch]
        if (HRaise.SplattingLayer == 0)
            OutputMap[CurPixel.xy] += float4(1.0f / 255.0f, 0, 0, 0) * factor;
        else if (HRaise.SplattingLayer == 1)
            OutputMap[CurPixel.xy] += float4(0, 1.0f / 255.0f, 0, 0) *factor;
        else if (HRaise.SplattingLayer == 2)                          
            OutputMap[CurPixel.xy] += float4(0, 0, 1.0f / 255.0f, 0) * factor;

    }
}

///////////////////////////////////////////////////////////////////////////////

[numthreads(1024, 1, 1)]
void SmoothingCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);

    float4 color = float4(0, 0, 0, 0.0f);
    
    [branch]
    if (HRaise.BrushType == 1)
    {
        if (CurPixel.x >= HRaise.Box.x
            && CurPixel.x < HRaise.Box.y
            && CurPixel.y < HRaise.Box.w
            && CurPixel.y >= HRaise.Box.z)
        {
            int r = sqrt(HRaise.Radius);
            int2 uv = CurPixel.xy;
            for (int i = -r; i < r + 1; i++)
                for (int j = -r; j < r + 1; j++)
                {
                    color += AlphaMap.Load(int3(uv + int2(i, j), 0));
                }
            color /= (2 * r + 1) * (2 * r + 1);
            OutputMap[CurPixel.xy] *= float4(1, 1, 1, 0);
            OutputMap[CurPixel.xy] += float4(0, 0, 0, color.a);
        }
    }
    else if (HRaise.BrushType == 2)
    {
          
        float len = length(CurPixel.xy - HRaise.Position);

        if (len < HRaise.Radius)
        {
            int r = sqrt(HRaise.Radius);
            int2 uv = CurPixel.xy;
            for (int i = -r; i < r + 1; i++)
                for (int j = -r; j < r + 1; j++)
                {
                    color += AlphaMap.Load(int3(uv + int2(i, j), 0));
                }
            color /= (2 * r + 1) * (2 * r + 1);
            OutputMap[CurPixel.xy] *= float4(1, 1, 1, 0);
            OutputMap[CurPixel.xy] += float4(0, 0, 0, color.a);
        }

    }
}

[numthreads(1024, 1, 1)]
void NoiseCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    float4 color = float4(0, 0, 0, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
    
    
    int x = CurPixel.x - HRaise.Box.x;
    int y = CurPixel.y - HRaise.Box.z;
    
    [branch]
    if (HRaise.BrushType == 1)
    {
        if (CurPixel.x >= HRaise.Box.x
            && CurPixel.x < HRaise.Box.y
            && CurPixel.y < HRaise.Box.w
            && CurPixel.y >= HRaise.Box.z)
        {
            uint3 PerlinPixel = uint3(x, y, 0);
            color=PerlinMap.Load(PerlinPixel);
            float height = color.r + color.g + color.b + color.a;
            height /= 4.0f;
            height /= 255.0f;
            
            OutputMap[CurPixel.xy] += float4(0, 0, 0, height) * HRaise.Rate;
        }
    }
    else if (HRaise.BrushType == 2)
    {
        float len = length(CurPixel.xy - HRaise.Position);

        if (len < HRaise.Radius)
        {
            uint3 PerlinPixel = uint3(x, y, 0);
            color = PerlinMap.Load(PerlinPixel);
            float height = color.r + color.g + color.b+color.a;
            height /= 4.0f;
            height /= 255.0f;
            OutputMap[CurPixel.xy] += float4(0, 0, 0, height) * HRaise.Rate;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
[numthreads(1024, 1, 1)]
void InitCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    //= float4(0, 0, 0, 0);    color +
    float4 color = AlphaMap.Load(CurPixel);
    OutputMap[CurPixel.xy] = float4(0, 0, 0, color.b);
}


[numthreads(1024, 1, 1)]
void InitNoAlphaCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(DispatchThreadID.x % HRaise.Res.x, DispatchThreadID.x / HRaise.Res.x, 0);
    OutputMap[CurPixel.xy] = AlphaMap.Load(CurPixel);
}

///////////////////////////////////////////////////////////////////////////////////
//UV Picking
Texture2D<float4> Terrain;
struct Col_Output
{
    float4 Pickcolor;
};
RWStructuredBuffer<Col_Output> OutputPickColor;
float3 MousePos;
[numthreads(1, 1, 1)]
void UVPick()
{
    uint3 PickPos = uint3(MousePos.x, MousePos.y, 0);    
    OutputPickColor[0].Pickcolor = Terrain.Load(PickPos);
    
}
///////////////////////////////////////////////////////////////////////////////////
technique11 T0_Raise
{
    pass P0_QuadRaise
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, QuadRaiseCS()));
    }

    pass P1_CircleRaise
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CircleRaiseCS()));
    }

    pass P2_SlopeRaise
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, SlopeRaiseCS()));
    }
}

technique11 T1_Splatting
{
    pass P0_QuadSplatting
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, QuadSplattingCS()));
    }

    pass P1_CircleSplatting
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CircleSplattingCS()));
    }
}
technique11 T2_OPTION
{
    pass P0_SmoothingCS
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, SmoothingCS()));
    }
    pass P1_NoiseCS
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, NoiseCS()));
    }
}

technique11 T3_ETC
{
    pass P0_Init
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, InitCS()));
    }
    pass P1_InitNoAlpha
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, InitNoAlphaCS()));
    }
 
    pass P2_UVPick
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, UVPick()));
    }
}

