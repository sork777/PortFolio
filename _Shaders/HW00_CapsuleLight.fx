#include "HW00_common.fx"
#include "HW00_DeferredFunc.fx"

/////////////////////////////////////////////////////////////////////////////
// Capsule Light
/////////////////////////////////////////////////////////////////////////////

struct sCapsuleLightDesc
{
    float4 color;

    float3 Position;
    float RangeRcp;

    float3 Direction;
    float Length;

    float4x4 LightProjection;
    float HalfLen;
    float Range;    
};

cbuffer CB_CapsuleLights
{
    sCapsuleLightDesc cbCapsuleLight;
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 CapsuleLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_ConstantCapsuleOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_ConstantCapsuleOutput CapsuleLightConstantHS()
{
    HS_ConstantCapsuleOutput Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_CapsuleOutput
{
    float4 CapsuleDir : POSITION;
};

static const float4 CapsuelDir[2] =
{
    float4(1.0, 1.0, 1.0, 1.0),
	float4(-1.0, 1.0, -1.0, 1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CapsuleLightConstantHS")]
HS_CapsuleOutput CapsuleLightHS(uint PatchID : SV_PrimitiveID)
{
    HS_CapsuleOutput Output;

    Output.CapsuleDir = CapsuelDir[PatchID];

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_CapsuleOutput
{
    float4 Position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

#define CylinderPortion 0.2
#define SpherePortion   (1.0 - CylinderPortion)
#define ExpendAmount    (1.0 + CylinderPortion)

[domain("quad")]
DS_CapsuleOutput CapsuleLightDS(HS_ConstantCapsuleOutput input, float2 UV : SV_DomainLocation, const OutputPatch<HS_CapsuleOutput, 4> quad)
{
	// Transform the UV's into clip-space
    float2 posClipSpace = UV.xy * float2(2.0, -2.0) + float2(-1.0, 1.0);

	// Find the vertex offsets based on the UV
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Force the cone vertices to the mesh edge
    float2 posClipSpaceNoCylAbs = saturate(posClipSpaceAbs * ExpendAmount);
    float maxLenNoCapsule = max(posClipSpaceNoCylAbs.x, posClipSpaceNoCylAbs.y);
    float2 posClipSpaceNoCyl = sign(posClipSpace.xy) * posClipSpaceNoCylAbs;

	// Convert the positions to half sphere with the cone vertices on the edge
    float3 halfSpherePos = normalize(float3(posClipSpaceNoCyl.xy, 1.0 - maxLenNoCapsule));

	// Find the offsets for the cone vertices (0 for cone base)
    float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Apply the range
    halfSpherePos *= cbCapsuleLight.Range;

	// Offset the cone vertices to thier final position
    float4 posLS = float4(halfSpherePos.xy, halfSpherePos.z + cbCapsuleLight.HalfLen - cylinderOffsetZ * cbCapsuleLight.HalfLen, 1.0);

	// Move the vertex to the selected capsule side
    posLS *= quad[0].CapsuleDir;

	// Transform all the way to projected space and generate the UV coordinates
    DS_CapsuleOutput Output;
    Output.Position = mul(posLS, cbCapsuleLight.LightProjection);
    Output.cpPos = Output.Position.xy / Output.Position.w;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////////////////////////////////
float3 CalcCapsule(float3 position, DeferredMaterial material)
{
    float3 ToEye = ViewPosition() - position;
   
   // Find the shortest distance between the pixel and capsules segment
    float3 ToCapsuleStart = position - cbCapsuleLight.Position;
    float DistOnLine = dot(ToCapsuleStart, cbCapsuleLight.Direction) / cbCapsuleLight.Length;
    DistOnLine = saturate(DistOnLine) * cbCapsuleLight.Length;
    float3 PointOnLine = cbCapsuleLight.Position + cbCapsuleLight.Direction * DistOnLine;
    float3 ToLight = PointOnLine - position;
    float DistToLight = length(ToLight);
   
   // Phong diffuse
    ToLight /= DistToLight; // Normalize
    float NDotL = saturate(dot(ToLight, material.normal));
    float3 finalColor = material.diffuseColor.rgb * NDotL;
   
   // Blinn specular
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(HalfWay, material.normal));
    finalColor += pow(NDotH, material.specPow) * material.specIntensity;
   
   // Attenuation
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbCapsuleLight.RangeRcp);
    float Attn = DistToLightNorm * DistToLightNorm;
    finalColor *= cbCapsuleLight.color.rgb * Attn;
   
    return finalColor;
}

float4 CapsuleLightPS(DS_CapsuleOutput In) : SV_TARGET
{
	// Unpack the GBuffer
    SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcCapsule(position, mat);

	// Return the final color
    return float4(finalColor, 1.0);
}

technique11 T0
{
    ///////////////////////
    /*    Capsule Light  */
    ///////////////////////
    pass P0
    {
        SetRasterizerState(NoDepthClipFrontRS);
        SetBlendState(AdditiveAlphaBlend, float4(0, 0, 0, 0), 0xFF);
        SetDepthStencilState(NoDepthWGreateEqualDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, CapsuleLightVS()));
        SetHullShader(CompileShader(hs_5_0, CapsuleLightHS()));
        SetDomainShader(CompileShader(ds_5_0, CapsuleLightDS()));
        SetPixelShader(CompileShader(ps_5_0, CapsuleLightPS()));
    }
}