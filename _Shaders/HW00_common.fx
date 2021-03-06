#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

/////////////////////////////////////////////////////////////////////////////
// Deferred
/////////////////////////////////////////////////////////////////////////////
float4 PerspectiveValues;

Texture2D DepthTexture;
Texture2D ColorSpecIntTexture;
Texture2D NormalTexture;
Texture2D SpecPowTexture;
Texture2D AOTexture;


Texture2D AtmosphereMap;
Texture2D DepthMap;

/////////////////////////////////////////////////////////////////////////////
SamplerComparisonState PCFSampler;

/////////////////////////////////////////////////////////////////////////////

float3 CalcWorldPos(float2 cpPos, float linearDepth)
{
    float4 position;
    position.xy = cpPos.xy * PerspectiveValues.xy * linearDepth;
    position.z = linearDepth;
    position.w = 1.0;
    return mul(position, ViewInverse).xyz;
}
/////////////////////////////////////////////////////////////////////////////

struct SURFACE_DATA
{
    float LinearDepth;
    float3 Color;
    float3 Normal;
    float3 SpecPow;
    float SpecIntensity;
};

struct DeferredMaterial
{
    float3 normal;
    float4 diffuseColor;
    float3 specPow;
    float specIntensity;
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
