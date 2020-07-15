#include "D:\GitHubPJ\PF\PortFolio\_Shaders/000_Math.fx"
#include "D:\GitHubPJ\PF\PortFolio\_Shaders/000_Status.fx"

cbuffer CB_PerFrame
{
    matrix View;
    matrix ViewInverse;
    matrix Projection;
    matrix VP;

    float Time;
};

cbuffer CB_World
{
    matrix World;
    matrix InvWorld;
};

Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;
Texture2D DisplaceMentMap;
Texture2D HeightMap;
//중요 2D로 하면 샘플링 안됨
TextureCube SkyCubeMap;
/* DSV에 대한 SRV가 들어올거임 */ 
Texture2D ShadowMap;

float4 Planes[6];

///////////////////////////////////////////////////////////////////////////////
/*                            Basic Functions                                */
float4 WorldPosition(float4 position)
{
    return mul(position, World);
}
float4 ViewProjection(float4 position)
{
    //position = mul(position, View);
    return mul(position, VP);
}

float3 WorldNormal(float3 normal)
{
    return mul(normal, (float3x3) World);
}

float3 WorldTangent(float3 tangent)
{
    return mul(tangent, (float3x3) World);
}
float3 ViewPosition()
{
    //카메라
    return ViewInverse._41_42_43;
}

void Texture(inout float4 color, Texture2D t, float2 uv, SamplerState samp)
{
    color = color * t.Sample(samp, uv);
}
void Texture(inout float4 color, Texture2D t, float2 uv)
{
    Texture(color, t, uv, LinearSampler);
}

///////////////////////////////////////////////////////////////////////////////
float4 PerspectiveValues;

float ConvertZToLinearDepth(float depth)
{
    float linearDepth = PerspectiveValues.z / (depth + PerspectiveValues.w);
    return linearDepth;
}

float3 CalcWorldPos(float2 cpPos, float linearDepth)
{
    float4 position;
    position.xy = cpPos.xy * PerspectiveValues.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0;
    return mul(position, ViewInverse).xyz;
}
///////////////////////////////////////////////////////////////////////////////

struct Vertex
{
    float4 Position : POSITION0;
};

struct VertexNormal
{
    float4 Position : POSITION0;
    float3 Normal : NORMAL0;
};

struct VertexColor
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
};

struct VertexColorNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float3 Normal : NORMAL0;
};

struct VertexTexture
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
};

struct VertexTextureColor
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float4 Color : COLOR0;
};

struct VertexTextureColorNormal
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float4 Color : COLOR0;
    float3 Normal : NORMAL0;
};

struct VertexTextureNormal
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float3 Normal : NORMAL0;
};

struct VertexColorTextureNormal
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
    float2 Uv : Uv0;
    float3 Normal : NORMAL0;
};

struct VertexTextureNormalBlend
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float3 Normal : NORMAL0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;
};

struct VertexTextureNormalTangent
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float3 Normal : NORMAL0;
    float3 Tangent : TANGENT0;
};

struct VertexTextureNormalTangentBlend
{
    float4 Position : POSITION0;
    float2 Uv : Uv0;
    float3 Normal : NORMAL0;
    float4 BlendIndices : BLENDINDICES0;
    float4 BlendWeights : BLENDWEIGHTS0;
};

///////////////////////////////////////////////////////////////////////////////


cbuffer CB_Shadow
{
    matrix ShadowView;
    matrix ShadowProjection;

    float2 ShadowMapSize;
    float ShadowBias;

    uint ShadowQuality;
};


struct DepthOutput
{
    float4 Position : SV_Position0;
    float4 sPosition : Position1;
};

///////////////////////////////////////////////////////////////////////////////

struct MeshOutput
{
    float4 Position : SV_Position0;
    float4 wvpPosition : Position1;
    float3 oPosition : Position2;
    float3 wPosition : Position3;
    float4 sPosition : Position4;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;

    uint ID : Id0;
    float4 Clip : SV_ClipDistance;
};

struct MeshTessOutput
{
    float4 Position : Position0;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
};
struct GeometryOutput
{
    float4 Position : SV_Position0;
    float4 wvpPosition : Position1;
    float3 oPosition : Position2;
    float3 wPosition : Position3;
    float4 sPosition : Position4;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
    uint TargetIndex : SV_RenderTargetArrayIndex; //ArraySize의 번호
};



///////////////////////////////////////////////////////////////////////////////
// Vertex / Pixel
///////////////////////////////////////////////////////////////////////////////
#define P_VP(name, vs, ps) \
pass name \
{ \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_VP(name, rs, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_BS_VP(name, bs, vs, ps) \
pass name \
{ \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_VP(name, dss, vs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_BS_VP(name, dss,bs, vs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_VP(name, rs, dss, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_BS_VP(name, rs, dss,bs, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_BS_VP(name, rs, bs, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

///////////////////////////////////////////////////////////////////////////////
// Vertex / Geometry / Pixel
///////////////////////////////////////////////////////////////////////////////
#define P_VGP(name, vs, gs, ps) \
pass name \
{ \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_VGP(name, rs, vs, gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_BS_VGP(name, bs, vs,gs, ps) \
pass name \
{ \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_VGP(name, dss, vs, gs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_BS_VGP(name, rs, bs, vs,gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_VGP(name, rs, dss, vs, gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_BS_VGP(name, dss, bs, vs, gs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_BS_VGP(name, rs,dss, bs, vs, gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}
/* 일단 미러용 */

#define P_DSS_Ref_VP(name, dss,ref,vs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, ref); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_Ref_BS_VP(name, dss,ref,bs, vs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, ref); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_Ref_BS_VP(name, rs, dss,ref,bs, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, ref); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_Ref_VP(name, rs, dss,ref, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, ref); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

///////////////////////////////////////////////////////////////////////////////
// Vertex / Tessellation / Pixel
///////////////////////////////////////////////////////////////////////////////
#define P_VTP(name, vs, hs, ds, ps) \
pass name \
{ \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_VTP(name, rs, vs, hs, ds, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_BS_VTP(name, bs, vs, hs, ds, ps) \
pass name \
{ \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_VTP(name, dss, vs, hs, ds, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_DSS_VTP(name, rs, dss, vs, hs, ds, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_BS_VTP(name, rs, bs, vs, hs, ds, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_DSS_BS_VTP(name, dss, bs, vs, hs, ds, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetHullShader(CompileShader(hs_5_0, hs())); \
    SetDomainShader(CompileShader(ds_5_0, ds())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}