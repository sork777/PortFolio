#include "HW00_common.fx"
#include "HW00_DeferredFunc.fx"

/////////////////////////////////////////////////////////////////////////////
// Spot Light
/////////////////////////////////////////////////////////////////////////////

struct sSpotLightDesc
{
    float4 color;

    float3 Position;
    float RangeReverse;

    float3 Direction;
    float SpotCosOuterCone;

    float4x4 LightProjection;

    float SpotCosConeAttRange;

    float SinAngle;
    float CosAngle;
	
};

cbuffer CB_SpotLights
{
    sSpotLightDesc cbSpotLight;
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 SpotLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_Const_SpotOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_Const_SpotOutput SpotLightConstantHS()
{
    HS_Const_SpotOutput Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_SPOT_OUTPUT
{
    float4 Position : POSITION0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("SpotLightConstantHS")]
HS_SPOT_OUTPUT SpotLightHS()
{
    HS_SPOT_OUTPUT Output;

    Output.Position = float4(0.0, 0.0, 0.0,1.0);

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_SPOT_OUTPUT
{
    float4 Position : SV_POSITION0;
    float3 PositionXYW : TEXCOORD0;
};

#define CylinderPortion 0.2
#define ExpendAmount    (1.0 + CylinderPortion)

[domain("quad")]
DS_SPOT_OUTPUT SpotLightDS(HS_Const_SpotOutput input, float2 UV : SV_DomainLocation, const OutputPatch<HS_SPOT_OUTPUT, 4> quad)
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

	// Scale the sphere to the size of the cones rounded base
    halfSpherePos = normalize(float3(halfSpherePos.xy * cbSpotLight.SinAngle, cbSpotLight.CosAngle));

	// Find the offsets for the cone vertices (0 for cone base)
    float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Offset the cone vertices to thier final position
    float4 posLS = float4(halfSpherePos.xy * (1.0 - cylinderOffsetZ), halfSpherePos.z - cylinderOffsetZ * cbSpotLight.CosAngle, 1.0);

	// Transform all the way to projected space and generate the UV coordinates
    DS_SPOT_OUTPUT Output;
    Output.Position = mul(posLS, cbSpotLight.LightProjection);
    Output.PositionXYW = Output.Position.xyw;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////////////////////////////////
Texture2D SpotShadowMapTexture;
matrix ToShadowMap;
uint bDrawSpotShadow;

float4 VS_Mesh_Spot(VertexMesh input) : SV_Position
{
    MeshOutput output;
    return mul(WorldPosition(input.Position), ToShadowMap);

}

float4 VS_Model_Spot(VertexModel input) : SV_Position
{
    MeshOutput output;
    SetModelWorld(World, input);
    return mul(WorldPosition(input.Position), ToShadowMap);

}
// Shadow PCF calculation helper function
float SpotShadowPCF(float3 position)
{
	// Transform the world position to shadow projected space
    float4 posShadowMap = mul(float4(position, 1.0), ToShadowMap);

	// Transform the position to shadow clip space
    float3 UVD = posShadowMap.xyz / posShadowMap.w;

	// Convert to shadow map UV values
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;

	// Compute the hardware PCF value
    return SpotShadowMapTexture.SampleCmpLevelZero(PCFSampler, UVD.xy, UVD.z);
}

float3 CalcSpot(float3 position, DeferredMaterial material)
{
    float3 ToLight = cbSpotLight.Position - position;
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
    finalColor += pow(NDotH, material.specPow) * material.specIntensity;

	// Cone attenuation
    float cosAng = dot(cbSpotLight.Direction, ToLight);
    float conAtt = saturate((cosAng - cbSpotLight.SpotCosOuterCone) / cbSpotLight.SpotCosConeAttRange);
    conAtt *= conAtt;
   
    float shadowAtt=1.0f;
    if (bDrawSpotShadow == 1)
    {
        shadowAtt = SpotShadowPCF(position);
    }

	// Attenuation
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbSpotLight.RangeReverse);
    float Attn = DistToLightNorm * DistToLightNorm;
    finalColor *= cbSpotLight.color.rgb * Attn * conAtt * shadowAtt*5;
   
	// Return the fianl color
    return finalColor;
}
