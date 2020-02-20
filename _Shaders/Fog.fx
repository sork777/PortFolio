#include "000_Header.fx"
#include "000_Light.fx"

cbuffer CB_Render2D
{
    matrix View2D;
    matrix Projection2D;
};

///////////////////////////////////////////////////////////////////////////////
/*                            Fog                                           */
Texture2D DepthMap;
cbuffer CB_Fog
{
    float4 FogColor;
    float2 FogDistance;
    float FogDensity;
    uint FogType;
};
float4 LinearFogBlend(float4 color, float dist)
{
    dist = ( FogDistance.y-dist) / ( FogDistance.y +  FogDistance.x);
    dist = saturate(dist);
    float factor = pow(dist, FogDensity);
    factor = saturate(factor);
    return float4(lerp(FogColor.rgb, color.rgb, factor), 1);
}

float4 ExpFogBlend(float4 color, float dist)
{
    dist = (dist- FogDistance.y) / ( FogDistance.y +  FogDistance.x);
    
    dist = saturate(dist);
    float factor = exp(-dist * FogDensity);
    factor = saturate(factor);
    return float4(lerp(FogColor.rgb, color.rgb, factor), 1);
}

float4 Exp2FogBlend(float4 color, float dist)
{
    dist = (dist- FogDistance.y) / ( FogDistance.y +  FogDistance.x);
    dist = saturate(dist);
    
    float factor = exp(-(dist * FogDensity) * (dist * FogDensity));
    factor = saturate(factor);
    return float4(lerp(FogColor.rgb, color.rgb, factor), 1);
}

float4 NDiviaFog(float4 color, float3 wPos)
{
    float3 eyeToPixel = wPos - ViewPosition();
    float eyePosY = ViewPosition().y;
    float pixelDist = length(eyeToPixel);
    float3 eyeToPixelNorm = eyeToPixel / pixelDist;

	// Find the fog staring distance to pixel distance
    float fogDist = max(pixelDist - FogDistance.x, 0.0);

    float fFogHeightFalloff = FogDistance.y * 0.005f;
    
	// Distance based fog intensity
    float fogHeightDensityAtViewer = exp(-fFogHeightFalloff * eyePosY);
    float fogDistInt = fogDist * fogHeightDensityAtViewer;

	// Height based fog intensity
    float eyeToPixelY = eyeToPixel.y * (fogDist / pixelDist);
    float t = fFogHeightFalloff * eyeToPixelY;
    const float thresholdT = 0.01;
    float fogHeightInt = abs(t) > thresholdT ?
		(1.0 - exp(-t)) / t : 1.0;

	// Combine both factors to get the final factor
    float fogFinalFactor = exp(-FogDensity * fogDistInt * fogHeightInt);

	// Find the sun highlight and use it to blend the fog color
    float sunHighlightFactor = saturate(dot(eyeToPixelNorm, -GlobalLight.Direction));
    sunHighlightFactor = pow(sunHighlightFactor, 8.0);
    float3 fogFinalColor = lerp(FogColor, GlobalLight.Specular, sunHighlightFactor);

    return float4(lerp(fogFinalColor, color.rgb, fogFinalFactor), 1);
}

float4 CalcualteFogColor(float4 color, float3 wPos)
{
    float dist = distance(wPos, ViewPosition());
    if (FogType == 0)
        color = LinearFogBlend(color, dist);
    else if (FogType == 1)
        color = ExpFogBlend(color, dist);
    else if (FogType == 2)
        color = Exp2FogBlend(color, dist);
    else if (FogType == 3)
        color = NDiviaFog(color, wPos);
    
    return color;
}
//-----------------------------------------------------------------------------
// Pass0
//-----------------------------------------------------------------------------
struct VertexOutput
{
    float4 Position : SV_POSITION0;
    float2 Uv : Uv0;
    float2 Cpos : Uv0;
};

VertexOutput VS(VertexTexture input)
{
    VertexOutput output;

    output.Cpos = input.Position * 2.0f;
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, View2D);
    output.Position = mul(output.Position, Projection2D);
    output.Uv = input.Uv;

    return output;
}

float4 PS_Fog(VertexOutput input) : SV_TARGET0
{
    float2 uv = input.Uv;  

    float4 pixel = DiffuseMap.Sample(LinearSampler, uv);
    int3 location3 = int3(input.Cpos, 0);

    float depth = DepthMap.Sample(LinearSampler, uv).r;
    depth = ConvertZToLinearDepth(depth);
    float3 wPos = CalcWorldPos(input.Cpos, depth);
   
    pixel = CalcualteFogColor(pixel, wPos);
    return float4(pixel.rgb, 1);
}
//-----------------------------------------------------------------------------
// Techniques
//-----------------------------------------------------------------------------
DepthStencilState Depth
{
    DepthEnable = false;
};

technique11 T0
{
    P_DSS_VP(P0, Depth, VS, PS_Fog)
}