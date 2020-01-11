#include "000_Header.fx"
//-----------------------------------------------------------------------------------------
// Occlusion
//-----------------------------------------------------------------------------------------

Texture2D<float> DepthTex;
RWTexture2D<float> OcclusionRW;

struct OcculusionDesc
{
    uint2 Res;
};
cbuffer CB_Occlusion
{
    OcculusionDesc Occ;
}


[numthreads(1024, 1, 1)]
void Occlussion(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint3 CurPixel = uint3(dispatchThreadId.x % Occ.Res.x, dispatchThreadId.x / Occ.Res.x, 0);

	// Skip out of bound pixels
    if (CurPixel.y < Occ.Res.y)
    {
		// Get the depth
        float curDepth = DepthTex.Load(CurPixel);

		// Flag anything closer than the sky as occlusion
        OcclusionRW[CurPixel.xy].x = curDepth > 0.99;
    }
}

//-----------------------------------------------------------------------------------------
// Ray tracing
//-----------------------------------------------------------------------------------------

struct RayTraceDesc
{
    float2 SunPos;
    float InitDecay;
    float DistDecay;
    float3 RayColor;
    float MaxDeltaLen;
};
cbuffer CB_RayTrace
{
    RayTraceDesc RayTrace;
}

Texture2D<float> OcclusionTex;

const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
   float2(1.0, 1.0),
   float2(-1.0, -1.0),
   float2(1.0, -1.0),
    
};
const float2 arrUV[4] =
{
    float2(0.0, 0.0),
    float2(1.0, 0.0),
	float2(0.0, 1.0),
	float2(1.0, 1.0),
  
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

VS_OUTPUT RayTraceVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT Output;

    Output.Position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    Output.UV = arrUV[VertexID].xy;

    return Output;
}

static const int NUM_STEPS = 64;
static const float NUM_DELTA = 1.0 / 63.0f;
float4 RayTracePS(VS_OUTPUT In) : SV_TARGET
{
	// Find the direction and distance to the sun
    float2 dirToSun = (RayTrace.SunPos - In.UV);
    float lengthToSun = length(dirToSun);
    dirToSun /= lengthToSun;

	// Find the ray delta
    float deltaLen = min(RayTrace.MaxDeltaLen, lengthToSun * NUM_DELTA);
    float2 rayDelta = dirToSun * deltaLen;

	// Each step decay	
    float stepDecay = RayTrace.DistDecay * deltaLen;

	// Initial values
    float2 rayOffset = float2(0.0, 0.0);
    float decay = RayTrace.InitDecay;
    float rayIntensity = 0.0f;

	// Ray march towards the sun
    for (int i = 0; i < NUM_STEPS; i++)
    {
		// Sample at the current location
        float2 sampPos = In.UV + rayOffset;
        float fCurIntensity = OcclusionTex.Sample(LinearSampler, sampPos);
		
		// Sum the intensity taking decay into account
        rayIntensity += fCurIntensity * decay;

		// Advance to the next position
        rayOffset += rayDelta;

		// Update the decay
        decay = saturate(decay - stepDecay);
    }

    return float4(rayIntensity, 0.0, 0.0, 0.0);
}

//-----------------------------------------------------------------------------------------
// Combine results
//-----------------------------------------------------------------------------------------

Texture2D<float> LightRaysTex;

float4 CombinePS(VS_OUTPUT In) : SV_TARGET
{
	// Ge the ray intensity
    float rayIntensity = LightRaysTex.Sample(LinearSampler, In.UV);

	// Return the color scaled by the intensity
    return float4(RayTrace.RayColor * rayIntensity, 1.0);
}



technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, Occlussion()));
    }
    P_VP(P1, RayTraceVS, RayTracePS)
    P_BS_VP(P2, AdditiveAlphaBlend, RayTraceVS, CombinePS)
    //P_BS_VP(P4, AdditiveAlphaBlend, FullScreenQuadVS, FinalPassPS)

}