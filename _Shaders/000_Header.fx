#include "D:\GitHubPJ\PF\PortFolio\_Shaders/000_Math.fx"

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
//중요 2D로 하면 샘플링 안됨
TextureCube SkyCubeMap;
/* DSV에 대한 SRV가 들어올거임 */ 
Texture2D ShadowMap;

///////////////////////////////////////////////////////////////////////////////
/*                                 States                                    */
/*           Raster             */
RasterizerState NoneRS
{
    CullMode = None;
    MultisampleEnable = true;
};
RasterizerState NoDepthClipFrontRS
{
    CullMode = Front;
    DepthClipEnable = false;
};
RasterizerState NoDepthRS
{
    DepthClipEnable = false;
};
RasterizerState WireFrameRS
{
    FillMode = WireFrame;
    MultisampleEnable = true;
};

RasterizerState shadowRS
{
    CullMode = Front;
    //DepthBias = 6;
    //SlopeScaledDepthBias = 1.0f;
    DepthBias = 85;
    SlopeScaledDepthBias = 5.0f;
};
RasterizerState cascadeRS
{
    CullMode = Front;
    DepthBias = 6;
    SlopeScaledDepthBias = 1.0f;
    DepthClipEnable = false;
};

RasterizerState TrailRS
{
    CullMode = None;
};
///////////////////////////////////////////////////////////////////////////////
/*           Sampler             */
SamplerComparisonState ShadowSampler;

SamplerState Sampler;
SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 1;
    ComparisonFunc = Always;
};

SamplerState PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 1;
    ComparisonFunc = Always;
};

SamplerState CubeSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 1;
    ComparisonFunc = Never;
};
SamplerState HeightSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 1;
    ComparisonFunc = Never;
};
SamplerState GradientSampler
{
    Filter = ANISOTROPIC;

    MaxAnisotropy = 8;
    ComparisonFunc = Never;
};
SamplerState PerlinSampler
{
    Filter = ANISOTROPIC;
    
    MaxAnisotropy = 4;
    ComparisonFunc = Never;
};
SamplerState FresnelSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
    MaxAnisotropy = 1;    
    ComparisonFunc = Never;
};

SamplerState BiLinearClampSampler
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
    MaxAnisotropy = 16;
    ComparisonFunc = Always;
};
SamplerState TriLinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
    MaxAnisotropy = 16;
    ComparisonFunc = Always;
};
///////////////////////////////////////////////////////////////////////////////
/*          Blend            */

BlendState TransparentBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = INV_SRC_ALPHA;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = Zero;
    SrcBlendAlpha[0] = ONE;
    RenderTargetWriteMask[0] = 0x0f;
};
BlendState OpaqueBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = Zero;
    SrcBlend[0] = ONE;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = Zero;
    SrcBlendAlpha[0] = ONE;
    RenderTargetWriteMask[0] = 0x0f;
};

BlendState NoRenderTargetBlend
{
    BlendEnable[0] = false;
    DestBlend[0] = Zero;
    SrcBlend[0] = One;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = Zero;
    SrcBlendAlpha[0] = ONE;
    RenderTargetWriteMask[0] = 0x00;
};

BlendState AlphaBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = INV_SRC_ALPHA;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = Zero;
    DestBlendAlpha[0] = Zero;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState AdditiveAlphaBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = One;
    SrcBlend[0] = One;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = One;
    SrcBlendAlpha[0] = One;
    BlendOpAlpha[0] = Add;
    RenderTargetWriteMask[0] = 0x0f;
};

BlendState AdditiveBlend_Particle
{
    BlendEnable[0] = true;
    DestBlend[0] = One;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = One;
    SrcBlendAlpha[0] = SRC_ALPHA;
    BlendOpAlpha[0] = Add;
    
    RenderTargetWriteMask[0] = 0x0f;
};

BlendState TrailBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = One;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = Zero;
    SrcBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
    RenderTargetWriteMask[0] = 0x0f;
};
BlendState IllusionBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = One;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    DestBlendAlpha[0] = One;
    SrcBlendAlpha[0] = One;
    BlendOpAlpha[0] = Add;
    RenderTargetWriteMask[0] = 0x0f;
};
///////////////////////////////////////////////////////////////////////////////
/*          DepthStencil            */

DepthStencilState disableDSS
{
    DepthEnable = false;
    StencilEnable = false;
};

DepthStencilState mirrorDSS
{
    DepthEnable = true;
    DepthWriteMask = Zero;
    DepthFunc = Less;

    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;

    FrontFaceStencilFunc = Always;
    FrontFaceStencilPass = Replace;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Always;
    BackFaceStencilPass = Replace;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};

DepthStencilState reflectDSS
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = Less;

    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};


DepthStencilState noDoubleBlendDSS
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = Less;

    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Incr;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Incr;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};


DepthStencilState DepthMarkDSS
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = Less;

    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;

    FrontFaceStencilFunc = Always;
    FrontFaceStencilPass = Replace;
    FrontFaceStencilDepthFail = Replace;
    FrontFaceStencilFail = Replace;

    BackFaceStencilFunc = Always;
    BackFaceStencilPass = Replace;
    BackFaceStencilDepthFail = Replace;
    BackFaceStencilFail = Replace;
};

DepthStencilState NoDepthWLessDSS
{
    DepthEnable = true;
    DepthWriteMask = Zero;
    DepthFunc = Less;

    StencilEnable = true;

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};

DepthStencilState NoDepthWGreateEqualDSS
{
    DepthEnable = true;
    DepthWriteMask = Zero;
    DepthFunc = Greater_Equal;

    StencilEnable = true;

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};

DepthStencilState ShadowGenDSS
{
    DepthEnable = true;
    DepthWriteMask = All;
    DepthFunc = Less;

    StencilEnable = false;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;

    FrontFaceStencilFunc = Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilDepthFail = Keep;
    FrontFaceStencilFail = Keep;

    BackFaceStencilFunc = Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilDepthFail = Keep;
    BackFaceStencilFail = Keep;
};
DepthStencilState ParticleDSS
{
    DepthEnable = true;
    DepthWriteMask = Zero;
    DepthFunc = Always;

    StencilEnable = false;
    StencilReadMask = 0x00;
    StencilWriteMask = 0x00;
};
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