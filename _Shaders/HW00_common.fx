#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

/////////////////////////////////////////////////////////////////////////////
// Deferred
/////////////////////////////////////////////////////////////////////////////

Texture2D DepthTexture;
Texture2D ColorSpecIntTexture;
Texture2D NormalTexture;
Texture2D SpecPowTexture;
Texture2D EmissiveTexture;
Texture2D AOTexture;


Texture2D AtmosphereMap;
Texture2D DepthMap;

/////////////////////////////////////////////////////////////////////////////
SamplerComparisonState PCFSampler;

/////////////////////////////////////////////////////////////////////////////

struct SURFACE_DATA
{
    float3 Color;
    float LinearDepth;
    float3 Normal;
    float3 SpecPow;
    float SpecIntensity;
    float4 Emissive;
};

struct DeferredMaterial
{
    float3 normal;
    float4 diffuseColor;
    float3 specPow;
    float specIntensity;
    float4 emissive;
};

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
   float2(1.0, 1.0),
   float2(-1.0, -1.0),
   float2(1.0, -1.0),
};
static const float2 arrUV[4] =
{
	float2(0.0, 0.0),
    float2(1.0, 0.0),
	float2(0.0, 1.0),
	float2(1.0, 1.0),
};

/////////////////////////////////////////////////////////////////////////////
// constants
/////////////////////////////////////////////////////////////////////////////
