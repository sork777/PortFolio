#include "HW00_common.fx"
//-----------------------------------------------------------------------------------------
// Down Scale
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Compute shader
//-----------------------------------------------------------------------------------------
Texture2D HDRTex;

StructuredBuffer<float> AverageValues1D;
RWStructuredBuffer<float> AverageLum;

struct DownScaleDesc
{
    uint2 Res;
    uint Domain;
    uint GroupSize;
};
cbuffer CB_DownScale
{
    DownScaleDesc DS;
}


// Group shared memory to store the intermidiate results
groupshared float SharedPositions[1024];

/*static*/
const float4 LUM_FACTOR = float4(0.299, 0.587, 0.114, 0);

float DownScale4x4(uint2 CurPixel, uint groupThreadId)
{
    float avgLum = 0.0;

	// Skip out of bound pixels
    if (CurPixel.y < DS.Res.y)
    {
		// Sum a group of 4x4 pixels
        int3 nFullResPos = int3(CurPixel * 4, 0);
        float4 downScaled = float4(0.0, 0.0, 0.0, 0.0);
		[unroll]
        for (int i = 0; i < 4; i++)
        {
			[unroll]
            for (int j = 0; j < 4; j++)
            {
                downScaled += HDRTex.Load(nFullResPos, int2(j, i));
            }
        }
        downScaled /= 16.0;

		// Calculate the lumenace value for this pixel
        avgLum = dot(downScaled, LUM_FACTOR);

		// Write the result to the shared memory
        SharedPositions[groupThreadId] = avgLum;
    }

	// Synchronize before next step
    GroupMemoryBarrierWithGroupSync();
	
    return avgLum;
}

float DownScale1024to4(uint dispatchThreadId, uint groupThreadId, float avgLum)
{
	// Expend the downscale code from a loop
	[unroll]
    for (uint groupSize = 4, step1 = 1, step2 = 2, step3 = 3; groupSize < 1024; groupSize *= 4, step1 *= 4, step2 *= 4, step3 *= 4)
    {
		// Skip out of bound pixels
        if (groupThreadId % groupSize == 0)
        {
			// Calculate the luminance sum for this step
            float stepAvgLum = avgLum;
            stepAvgLum += dispatchThreadId + step1 < DS.Domain ? SharedPositions[groupThreadId + step1] : avgLum;
            stepAvgLum += dispatchThreadId + step2 < DS.Domain ? SharedPositions[groupThreadId + step2] : avgLum;
            stepAvgLum += dispatchThreadId + step3 < DS.Domain ? SharedPositions[groupThreadId + step3] : avgLum;
		
			// Store the results
            avgLum = stepAvgLum;
            SharedPositions[groupThreadId] = stepAvgLum;
        }

		// Synchronize before next step
        GroupMemoryBarrierWithGroupSync();
    }

    return avgLum;
}

void DownScale4to1(uint dispatchThreadId, uint groupThreadId, uint groupId, float avgLum)
{
    if (groupThreadId == 0)
    {
		// Calculate the average lumenance for this thread group
        float fFinalAvgLum = avgLum;
        fFinalAvgLum += dispatchThreadId + 256 < DS.Domain ? SharedPositions[groupThreadId + 256] : avgLum;
        fFinalAvgLum += dispatchThreadId + 512 < DS.Domain ? SharedPositions[groupThreadId + 512] : avgLum;
        fFinalAvgLum += dispatchThreadId + 768 < DS.Domain ? SharedPositions[groupThreadId + 768] : avgLum;
        fFinalAvgLum /= 1024.0;

		// Write the final value into the 1D UAV which will be used on the next step
        AverageLum[groupId] = fFinalAvgLum;
    }
}

[numthreads(1024, 1, 1)]
void DownScaleFirstPass(uint3 dispatchThreadId : SV_DispatchThreadID, uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
{
    uint2 CurPixel = uint2(dispatchThreadId.x % DS.Res.x, dispatchThreadId.x / DS.Res.x);

	// Reduce a group of 16 pixels to a single pixel and store in the shared memory
    float avgLum = DownScale4x4(CurPixel, groupThreadId.x);

	// Down scale from 1024 to 4
    avgLum = DownScale1024to4(dispatchThreadId.x, groupThreadId.x, avgLum);

	// Downscale from 4 to 1
    DownScale4to1(dispatchThreadId.x, groupThreadId.x, groupId.x, avgLum);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Second pass - convert the 1D average values into a single value
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_GROUPS 64

// Group shared memory to store the intermidiate results
groupshared float SharedAvgFinal[MAX_GROUPS];

[numthreads(MAX_GROUPS, 1, 1)]
void DownScaleSecondPass(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	// Fill the shared memory with the 1D values
    float avgLum = 0.0;
    if (dispatchThreadId.x < DS.GroupSize)
    {
        avgLum = AverageValues1D[dispatchThreadId.x];
    }
    SharedAvgFinal[dispatchThreadId.x] = avgLum;

    GroupMemoryBarrierWithGroupSync(); // Sync before next step

	// Downscale from 64 to 16
    if (dispatchThreadId.x % 4 == 0)
    {
		// Calculate the luminance sum for this step
        float stepAvgLum = avgLum;
        stepAvgLum += dispatchThreadId.x + 1 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 1] : avgLum;
        stepAvgLum += dispatchThreadId.x + 2 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 2] : avgLum;
        stepAvgLum += dispatchThreadId.x + 3 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 3] : avgLum;
		
		// Store the results
        avgLum = stepAvgLum;
        SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
    }

    GroupMemoryBarrierWithGroupSync(); // Sync before next step

	// Downscale from 16 to 4
    if (dispatchThreadId.x % 16 == 0)
    {
		// Calculate the luminance sum for this step
        float stepAvgLum = avgLum;
        stepAvgLum += dispatchThreadId.x + 4 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 4] : avgLum;
        stepAvgLum += dispatchThreadId.x + 8 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 8] : avgLum;
        stepAvgLum += dispatchThreadId.x + 12 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 12] : avgLum;

		// Store the results
        avgLum = stepAvgLum;
        SharedAvgFinal[dispatchThreadId.x] = stepAvgLum;
    }

    GroupMemoryBarrierWithGroupSync(); // Sync before next step

	// Downscale from 4 to 1
    if (dispatchThreadId.x == 0)
    {
		// Calculate the average luminace
        float fFinalLumValue = avgLum;
        fFinalLumValue += dispatchThreadId.x + 16 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 16] : avgLum;
        fFinalLumValue += dispatchThreadId.x + 32 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 32] : avgLum;
        fFinalLumValue += dispatchThreadId.x + 48 < DS.GroupSize ? SharedAvgFinal[dispatchThreadId.x + 48] : avgLum;
        fFinalLumValue /= 64.0;

		// Store the final value
        AverageLum[0] = max(fFinalLumValue, 0.0001);

    }
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
StructuredBuffer<float> AvgLum;
//-----------------------------------------------------------------------------------------
// Vertex shader
//-----------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position : SV_Position; // vertex position 
    float2 UV : TEXCOORD0;
};

VS_OUTPUT FullScreenQuadVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT Output;

    Output.Position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    Output.UV = arrUV[VertexID].xy;
    
    return Output;
}

//-----------------------------------------------------------------------------------------
// Pixel shader
//-----------------------------------------------------------------------------------------

struct FinalPassDesc
{
	// Tone mapping
    float MiddleGrey;
    float LumWhiteSqr;
};
cbuffer CB_FinalPass
{
    FinalPassDesc FP;
}


float3 ToneMapping(float3 HDRColor)
{
	// Find the luminance scale for the current pixel
    float LScale = dot(HDRColor, LUM_FACTOR.xyz);
    LScale *= FP.MiddleGrey / AvgLum[0];
    LScale = (LScale + LScale * LScale / FP.LumWhiteSqr) / (1.0 + LScale);
	
	// Apply the luminance scale to the pixels color
    return HDRColor * LScale;
}

float4 FinalPassPS(VS_OUTPUT In) : SV_TARGET0
{
	// Get the color sample
    float3 color = HDRTex.Sample(PointSampler, In.UV.xy).xyz;

	// Tone mapping
    color = ToneMapping(color);

	// Output the LDR value
    return float4(color, 1.0f);
}


technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, DownScaleFirstPass()));
    }
    pass P1
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, DownScaleSecondPass()));
    }

    P_VP(P2, FullScreenQuadVS, FinalPassPS)
    //P_DSS_Ref_VP(P3, DepthMarkDSS, 1, FullScreenQuadVS, FinalPassPS)
}