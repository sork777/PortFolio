#include "HW00_common.fx"
#include "HW00_DeferredFunc.fx"

/////////////////////////////////////////////////////////////////////////////
// Point Light
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// 변수들
/////////////////////////////////////////////////////////////////////////////

struct sPointLightDesc
{
    float4x4 LightProjection;
    float4 PointColor;
    float3 PointLightPos;
    float PointLightRangeRcp;
    float2 LightPerspectiveValues;
    float Specular;
};

cbuffer CB_PointLights
{
    sPointLightDesc cbPointLight;
};

static const float3 HemilDir[2] =
{
    float3(1.0, 1.0, 1.0),
	float3(-1.0, 1.0, -1.0)
};

matrix PL_CubeViewProj[6];
uint bDrawPointShadow;

/////////////////////////////////////////////////////////////////////////////
// Shadow 
/////////////////////////////////////////////////////////////////////////////
struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void PointShadowGenGS(triangle MeshOutput InPos[3] , inout TriangleStream<GS_OUTPUT> OutStream)
//void PointShadowGenGS(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
    for (int iFace = 0; iFace < 6; iFace++)
    {
        GS_OUTPUT output;

        output.RTIndex = iFace;

        for (int v = 0; v < 3; v++)
        {
            output.Pos = mul(InPos[v].Position, PL_CubeViewProj[iFace]);
            OutStream.Append(output);
        }
        OutStream.RestartStrip();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Light Calculation
/////////////////////////////////////////////////////////////////////////////
TextureCube PointShadowMapTexture;

float PointShadowPCF(float3 ToPixel)
{
    float3 ToPixelAbs = abs(ToPixel);
    float Z = max(ToPixelAbs.x, max(ToPixelAbs.y, ToPixelAbs.z));
    float Depth = (cbPointLight.LightPerspectiveValues.x * Z + cbPointLight.LightPerspectiveValues.y) / Z;
    return PointShadowMapTexture.SampleCmpLevelZero(PCFSampler, ToPixel, Depth);
}

float3 CalcPoint(float3 position, DeferredMaterial material)
{
    float3 ToLight = cbPointLight.PointLightPos - position;
    float3 ToEye = ViewPosition() - position;
    float DistToLight = length(ToLight);
 
   // Phong diffuse
    ToLight /= DistToLight; // Normalize
    
    float NDotL = saturate(dot(ToLight, material.normal));
    float3 finalColor = material.diffuseColor.rgb * NDotL;
 
   // Blinn specular
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(HalfWay, material.normal));

    finalColor += pow(NDotH, material.specPow.x) * material.specIntensity * cbPointLight.Specular;

   // 감쇄
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbPointLight.PointLightRangeRcp);
    float Attn = DistToLightNorm * DistToLightNorm;
    finalColor *= cbPointLight.PointColor.rgb;// * Attn;
 

    float shadowAtt = 1.0f;
    
    if (bDrawPointShadow == 1)
        shadowAtt = PointShadowPCF(position - cbPointLight.PointLightPos);

    return finalColor * shadowAtt;
}

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 PointLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}
/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PointLightConstantHS()
{
    HS_CONSTANT_DATA_OUTPUT Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_OUTPUT
{
    //반구 방향?
    float4 HemiDir : POSITION0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT PointLightHS(uint PatchID : SV_PrimitiveID)
{
    HS_OUTPUT Output;

    Output.HemiDir = float4(HemilDir[PatchID], 1);

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_OUTPUT
{
    float4 Position : SV_POSITION0;
    float2 cpPos : TEXCOORD0;
};

[domain("quad")]
DS_OUTPUT PointLightDS(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad)
{
    // Uv값을 클립공간의 좌표값으로
    float2 posClipSpace = UV * 2.0 - 1.0;

	// 중심에서 가장 먼 절대값의 거리?
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// 클립공간의 최종값 추출
    float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * quad[0].HemiDir.xyz);
    float4 posLS = float4(normDir.xyz, 1.0);
	
	// 프로젝션공간으로 변환 후 uv 생성
    DS_OUTPUT Output;
    Output.Position = mul(posLS, cbPointLight.LightProjection);
	// 클립공간 위치 저장
    Output.cpPos = Output.Position.xy / Output.Position.w;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Pixel shader - Point
/////////////////////////////////////////////////////////////////////////////

float4 PointLightPS(DS_OUTPUT input) : SV_TARGET0
{
	// Unpack the GBuffer
    SURFACE_DATA gbd = UnpackGBuffer_Loc(input.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 position = CalcWorldPos(input.cpPos, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcPoint(position, mat);
    return float4(finalColor, 1.0);
}
technique11 T0
{
    ///////////////////////
    /*    Point Light    */
    ///////////////////////
    pass P0
    {
        SetRasterizerState(NoDepthClipFrontRS);
        SetBlendState(AdditiveAlphaBlend, float4(0, 0, 0, 0), 0xFF);
        SetDepthStencilState(NoDepthWGreateEqualDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, PointLightVS()));
        SetHullShader(CompileShader(hs_5_0, PointLightHS()));
        SetDomainShader(CompileShader(ds_5_0, PointLightDS()));
        SetPixelShader(CompileShader(ps_5_0, PointLightPS()));
    }
}