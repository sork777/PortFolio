#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

float4 WaterClipPlane;
Texture2D ReflectionMap;
Texture2D RefractionMap;

cbuffer CB_PondBuffer
{
    matrix ReflectionMatrix;
    float4 RefractionColor;
    float2 NormalMapTile;

    float WaveTranslation;
    float WaveScale;
    float WaterShininess;
};

MeshOutput VS_Mesh_PreRender(VertexMesh input)
{
    MeshOutput output = VS_Mesh(input);
    output.Clip = dot(WorldPosition(input.Position), WaterClipPlane);

    return output;
}

MeshOutput VS_Model_PreRender(VertexModel input)
{
    MeshOutput output = VS_Model(input);
    output.Clip = dot(WorldPosition(input.Position), WaterClipPlane);

    return output;
}

MeshOutput VS_Animation_PreRender(VertexModel input)
{
    MeshOutput output = VS_Animation(input);
    output.Clip = dot(WorldPosition(input.Position), WaterClipPlane);

    return output;
}
float4 PS(MeshOutput input) : SV_Target0
{
    return VS_Shadow(input, VS_AllLight(input));
}
////////////////////////////////////////////////////////////////////////////////

struct VertexOutput
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float4 ReflectionPosition : Position2;
    float4 RefractionPosition : Position3;

    float2 Uv : Uv0;
    float2 Uv1 : Uv1;
};

VertexOutput VS_PondWater(VertexTexture input)
{
    VertexOutput output;
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = ViewProjection(output.Position);

    matrix reflection = mul(World, mul(ReflectionMatrix, Projection));
    output.ReflectionPosition = mul(input.Position, reflection);
    
    matrix reflaction = mul(World, mul(View, Projection));
    output.RefractionPosition = mul(input.Position, reflaction);

    //output.Uv = input.Uv;
    output.Uv = input.Uv / NormalMapTile.x;
    output.Uv1 = input.Uv / NormalMapTile.y;
    
    return output;
}


Texture2D WaveMap;
float4 PS_PondWater(VertexOutput input) : SV_Target0
{
    input.Uv.x += WaveTranslation;
    input.Uv1.y += WaveTranslation;

    //return float4(0, 0, 1, 0.5f);
    float4 normalMap = WaveMap.Sample(LinearSampler, input.Uv)*2.0f-1.0f;
    float4 normalMap2 = WaveMap.Sample(LinearSampler, input.Uv1) * 2.0f - 1.0f;

    float3 normal = normalize(normalMap.rgb + normalMap2.rgb);
    /* �ݻ�� �༮�� ������ § uv ��ǥ */
    float2 reflection;
    reflection.x = input.ReflectionPosition.x / input.ReflectionPosition.w * 0.5f + 0.5f;
    reflection.y = -input.ReflectionPosition.y / input.ReflectionPosition.w * 0.5f + 0.5f;
    reflection = reflection + (normal.xy * WaveScale);
    
    float2 refraction;
    refraction.x = input.RefractionPosition.x / input.RefractionPosition.w * 0.5f + 0.5f;
    refraction.y = -input.RefractionPosition.y / input.RefractionPosition.w * 0.5f + 0.5f;
    refraction = refraction + (normal.xy * WaveScale);
    
    float4 reflectColor = ReflectionMap.Sample(LinearSampler, reflection);
    float4 refractColor = RefractionMap.Sample(LinearSampler, refraction);
    refractColor = saturate(refractColor + RefractionColor);


    float3 viewDirection = normalize(ViewPosition() - input.wPosition);
    float3 heightView = viewDirection.yyy;
    float r = (1.2f - 1.0f) / (1.2f + 1.0f);
    float fresnel = max(0, min(1, r * (1 - r) * pow(1 - dot(normal, heightView), 2)));
    float3 color = lerp(reflectColor, refractColor, fresnel);


    
    float3 light = -GlobalLight.Direction;
    light.x *= -1.0f;

    float3 R = reflect(light, normal);
    float specular = saturate(dot(R, viewDirection));

    [flatten]
    if(specular>0.0f)
    {
        specular = pow(specular, WaterShininess);
        color = saturate(color + specular);
    }

    return float4(color, 0.5f);

}


RasterizerState RS
{
    CullMode = Front;
};

technique11 T0
{
    /* Depth */
    P_RS_VP(P0, RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1, RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2, RS, VS_Depth_Animation, PS_Depth)
   
    /* PreRender */
    P_VP(P3, VS_Mesh_PreRender, PS)
    P_VP(P4, VS_Model_PreRender, PS)
    P_VP(P5, VS_Animation_PreRender, PS)
    
    /* Render */
    P_VP(P6, VS_Mesh, PS)
    P_VP(P7, VS_Model, PS)
    P_VP(P8, VS_Animation, PS)
    
    P_BS_VP(P9,AlphaBlend, VS_PondWater, PS_PondWater)
}