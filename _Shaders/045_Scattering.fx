#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
#include "000_Sky.fx"

float HitOuterSphere(float3 position, float3 direction)
{
    float3 light = -position;

    float b = dot(light, direction);
    float c = dot(light, light);

    float d = c - b * b;
    float q = sqrt(OuterRadius * OuterRadius - d);

    return b + q;
}

float2 GetDensityRatio(float height)
{
    float altitude = (height - InnerRadius) * Scale;

    return exp(-altitude / RayleighMieScaleHeight);
}

float2 GetDistance(float3 p1, float3 p2)
{
    float2 opticalDepth = 0;

    float3 temp = p2 - p1;
    float far = length(temp);
    float3 direction = temp / far;


    float sampleLength = far / SampleCount;
    float scaledLength = sampleLength * Scale;

    float3 sampleRay = direction * sampleLength;
    p1 += sampleRay * 0.5f;

    for (int i = 0; i < SampleCount; i++)
    {
        float height = length(p1);
        opticalDepth += GetDensityRatio(height);

        p1 += sampleRay;
    }

    return opticalDepth * scaledLength;
}

///////////////////////////////////////////////////////////////////////////////

VertexOutput_Scatter VS_Scatter(VertexTexture input)
{
    VertexOutput_Scatter output;

    output.Position = input.Position;
    output.Uv = input.Uv;

    return output;
}

PixelOutput_Scatter PS_Scatter(VertexOutput_Scatter input)
{
    PixelOutput_Scatter output;
    
    float3 sunDirection = -normalize(GlobalLight.Direction);
    float2 uv = input.Uv;

    float3 pointPv = float3(0, InnerRadius + 1e-3f, 0.0f);
    float angleXZ = PI * uv.y;
    float angleY = 100.0f * uv.x * PI / 180.0f;

    float3 direction;
    direction.x = sin(angleY) * cos(angleXZ);
    direction.y = cos(angleY);
    direction.z = sin(angleY) * sin(angleXZ);
    direction = normalize(direction);

    float farPvPa = HitOuterSphere(pointPv, direction);
    float3 ray = direction;

    float3 pointP = pointPv;
    float sampleLength = farPvPa / SampleCount;
    float scaledLength = sampleLength * Scale;
    float3 sampleRay = ray * sampleLength;
    pointP += sampleRay * 0.5f;

    float3 rayleighSum = 0;
    float3 mieSum = 0;
    for (int i = 0; i < SampleCount; i++)
    {
        float pHeight = length(pointP);

        float2 densityRatio = GetDensityRatio(pHeight);
        densityRatio *= scaledLength;


        float2 viewerOpticalDepth = GetDistance(pointP, pointPv);

        float farPPc = HitOuterSphere(pointP, sunDirection);
        float2 sunOpticalDepth = GetDistance(pointP, pointP + sunDirection * farPPc);

        float2 opticalDepthP = sunOpticalDepth.xy + viewerOpticalDepth.xy;
        float3 attenuation = exp(-Kr4PI * InvWaveLength * opticalDepthP.x - Km4PI * opticalDepthP.y);

        rayleighSum += densityRatio.x * attenuation;
        mieSum += densityRatio.y * attenuation;

        pointP += sampleRay;
    }

    float3 rayleigh = rayleighSum * KrESun;
    float3 mie = mieSum * KmESun;

    rayleigh *= InvWaveLength;
    mie *= WaveLengthMie;

    output.R = float4(rayleigh, 1);
    output.M = float4(mie, 1);

    return output;
}
///////////////////////////////////////////////////////////////////////////////

VertexOutput_Dome VS_Dome(VertexTexture input)
{
    VertexOutput_Dome output;

    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);

    output.oPosition = -input.Position;
    output.Uv = input.Uv;

    return output;
}


Texture2D RayleighMap;
Texture2D MieMap;
Texture2D StarMap;

float4 PS_Dome(VertexOutput_Dome input) : SV_TARGET0
{
    float3 sunDirection = -normalize(GlobalLight.Direction);

    float temp = dot(sunDirection, input.oPosition) / length(input.oPosition);
    float temp2 = temp * temp;

    float3 rSamples = RayleighMap.Sample(LinearSampler, input.Uv);
    float3 mSamples = MieMap.Sample(LinearSampler, input.Uv);

    float3 color = 0;
    //color = GetRayleighPhase(temp2) * rSamples + GetMiePhase(temp, temp2) * mSamples;
    color = GetRayleighPhase(temp2) * rSamples + GetMiePhase(temp, temp2) * mSamples;
    /* �¾��� Ȯ ������ ���� ���� �ϴ� �κ�? */
    color = HDR(color);

    color += max(0, (1 - color.rgb)) * float3(0.05f, 0.05f, 0.1f);

    return float4(color, 1) + StarMap.Sample(LinearSampler, input.Uv) * saturate(StarIntensity);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VertexOutput_Moon VS_Moon(VertexTexture input)
{
    VertexOutput_Moon output;

    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);
    output.Uv = input.Uv;

    return output;
}


Texture2D MoonMap;

float4 PS_Moon(VertexOutput_Moon input) : SV_Target0
{
    float4 color = MoonMap.Sample(LinearSampler, input.Uv);
    color.a *= MoonAlpha;

    return color;
}

///////////////////////////////////////////////////////////////////////////////

VertexOutput_Cloud VS_Cloud(VertexTexture input)
{
    VertexOutput_Cloud output;

    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Uv = (input.Uv * CloudTiles);
    output.oUv = input.Uv;

    return output;
}

VertexOutput_Dome VS_CloudDome(VertexTexture input)
{
    VertexOutput_Dome output;
  
    output.Position = WorldPosition(input.Position);
    output.Position = ViewProjection(output.Position);

    output.oPosition = input.Position;
    output.Uv = input.Uv;

    return output;
}
float4 PS_Cloud(VertexOutput_Cloud input) : SV_Target0
{
    input.Uv = input.Uv * CloudTiles;

    float n = Noise(input.Uv + Time * CloudSpeed);
    float n2 = Noise(input.Uv * 2 + Time * CloudSpeed);
    float n3 = Noise(input.Uv * 4 + Time * CloudSpeed);
    float n4 = Noise(input.Uv * 8 + Time * CloudSpeed);
   
    float nFinal = n + (n2 / 2) + (n3 / 4) + (n4 / 8);
   
    float c = CloudCover - nFinal;
    if (c < 0) 
        c = 0;
 
    float density = 1.0 - pow(CloudSharpness, c);
    float4 color = density;
    //float4 color = CloudMap.Sample(LinearSampler,input.Uv);
    
    return color;
}

float4 PS_CloudDome(VertexOutput_Dome input) : SV_Target0
{
    input.Uv = input.Uv * CloudTiles * CloudTiles;
    
    float4 c1 = CloudMap.Sample(LinearSampler,input.Uv * 1 + Time * CloudSpeed);
    float4 c2 = CloudMap.Sample(LinearSampler,input.Uv * 2 + Time * CloudSpeed);
    float4 c3 = CloudMap.Sample(LinearSampler,input.Uv * 4 + Time * CloudSpeed);
    //float4 c4 = CloudMap.Sample(LinearSampler,input.Uv * 8 *2+ Time * CloudSpeed);
   
    float4 cFinal = c1 + (c2 / 2) + (c3 / 4);//    +(c4 / 8);
       
    float4 cf = CloudCover - cFinal;
    float4 density = pow(CloudSharpness, cf)-1.0f;
    float4 color = saturate(density);
    float3 sunDir = (-normalize(GlobalLight.Direction));
    float3 up = float3(0, 1, 0);
    float dayfactor = dot(sunDir, up);
    dayfactor = clamp(dayfactor, 0.3f, 1.0f);
    color.rgb *= dayfactor;
    
    return color;
    //float4(1, 0, 0, 1);
}

///////////////////////////////////////////////////////////////////////////////
DepthStencilState DSS
{
    DepthEnable = false;
};

technique11 T0
{
    P_VP(P0, VS_Scatter, PS_Scatter)
    P_DSS_VP(P1, DSS, VS_Dome, PS_Dome)
    P_DSS_BS_VP(P2, DSS, AlphaBlend, VS_Cloud, PS_Cloud)
    P_BS_VP(P3, AlphaBlend, VS_Moon, PS_Moon)
    P_DSS_BS_VP(P4, DSS, AlphaBlend2, VS_CloudDome, PS_CloudDome)
    P_RS_DSS_BS_VP(P5, WireFrameRS, DSS, AlphaBlend2, VS_CloudDome, PS_CloudDome)
    //P_RS_DSS_BS_VP(P4, WireFrameRS,DSS, AdditiveAlphaBlend, VS_CloudDome, PS_CloudDome)
}