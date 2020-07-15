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

BlendState AlphaBlend2
{
    BlendEnable[0] = true;
    DestBlend[0] = One;
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