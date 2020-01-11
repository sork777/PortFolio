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
};

Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;

TextureCube SkyCubeMap;

///////////////////////////////////////////////////////////////////////////////

float4 WorldPosition(float4 position)
{
    return mul(position, World);
}

float4 ViewProjection(float4 position)
{
    return mul(position, VP);

    //position = mul(position, View);
    //return mul(position, Projection);
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
    return ViewInverse._41_42_43;
}

///////////////////////////////////////////////////////////////////////////////

struct MeshOutput
{
    float4 Position : SV_Position0;
    float4 wvpPosition : Position1;
    float3 oPosition : Position2;
    float3 wPosition : Position3;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
};

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
    float2 Uv : UV0;
};

struct VertexTextureNormal
{
    float4 Position : POSITION0;
    float2 Uv : UV0;
    float3 Normal : NORMAL0;
};

///////////////////////////////////////////////////////////////////////////////

SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};

RasterizerState FillMode_WireFrame
{
    FillMode = Wireframe;
};

RasterizerState FrontCounterClockwise_True
{
    FrontCounterClockwise = true;
};

DepthStencilState DepthEnable_False
{
    DepthEnable = false;
};

BlendState AlphaBlend
{
    BlendEnable[0] = true;
    DestBlend[0] = INV_SRC_ALPHA;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = Zero;
    DestBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
    
    RenderTargetWriteMask[0] = 15;
};

BlendState AlphaBlend_AlphaToCoverage
{
    BlendEnable[0] = true;
    AlphaToCoverageEnable = true;
    DestBlend[0] = INV_SRC_ALPHA;
    SrcBlend[0] = SRC_ALPHA;
    BlendOp[0] = Add;

    SrcBlendAlpha[0] = Zero;
    DestBlendAlpha[0] = Zero;
    BlendOpAlpha[0] = Add;
    
    RenderTargetWriteMask[0] = 15;
};

///////////////////////////////////////////////////////////////////////////////

#define VS_GENERATE \
output.oPosition = input.Position.xyz; \
output.Position = WorldPosition(input.Position); \
output.wPosition = output.Position.xyz; \
\
output.Position = ViewProjection(output.Position); \
output.wvpPosition = output.Position; \
output.Normal = WorldNormal(input.Normal); \
output.Tangent = WorldTangent(input.Tangent); \
output.Uv = input.Uv;

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

#define P_RS_DSS_VP(name, rs, dss, vs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
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

#define P_DSS_BS_VP(name, dss, bs, vs, ps) \
pass name \
{ \
    SetDepthStencilState(dss, 0); \
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

#define P_BS_VGP(name, bs, vs, gs, ps) \
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

#define P_RS_DSS_VGP(name, rs, dss, vs, gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetDepthStencilState(dss, 0); \
    SetVertexShader(CompileShader(vs_5_0, vs())); \
    SetGeometryShader(CompileShader(gs_5_0, gs())); \
    SetPixelShader(CompileShader(ps_5_0, ps())); \
}

#define P_RS_BS_VGP(name, rs, bs, vs, gs, ps) \
pass name \
{ \
    SetRasterizerState(rs); \
    SetBlendState(bs, float4(0, 0, 0, 0), 0xFF); \
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