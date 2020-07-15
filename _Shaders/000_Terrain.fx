Texture2D BaseMap;
Texture2D LayerMap;
Texture2D AlphaMap;

Texture2D LayerMaps[3];


///////////////////////////////////////////////////////////////////////////////
/// Terrain
///////////////////////////////////////////////////////////////////////////////
struct VertexTerrain
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float4 Color : Color0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
};

struct TerrainOutput
{
    //sv는 픽셀 쉐이더에서 직접 접근 불가
    float4 Position : SV_Position0;
    float4 wvpPosition : Position1;
    float3 oPosition : Position2;
    float3 wPosition : Position3;
    float4 sPosition : Position4;

    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
    float4 Color : Color0;
};
///////////////////////////////////////////////////////////////////////////////
/// Terrain LOD
///////////////////////////////////////////////////////////////////////////////
struct TerrainLODDesc
{
    float MinDistance;
    float MaxDistance;
    float MinTessellation;
    float MaxTessellation;

    float TexelCellSpaceU;
    float TexelCellSpaceV;
    float WorldCellSpace;
    float TerrainHeightRatio;

    float2 TexScale;
    float CB_Terrain_Padding2[2];

    //float4 WorldFrustumPlanes[6];
};

cbuffer CB_Terrain
{
    TerrainLODDesc Lod;
};
struct Vertexinput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundY : BoundY0;
};

struct VertexOutput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundY : BoundY0;
   
};

struct ConstantHullOutput_Lod
{
    float Edge[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};


struct HullOutput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
};

struct DomainOutput_Lod
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
    float2 Uv : Uv0;
    float2 TiledUv : Uv1; //offset을 통해 몇장 반복하는지를 정하는 uv?
     
    //클립 컬 합쳐서 2개 까지만 가능
    float4 Clip : SV_ClipDistance0;
    float4 Clip2 : SV_ClipDistance1;
};
///////////////////////////////////////////////////////////////////////////////

TerrainOutput VS_Terrain(VertexTerrain input)
{
    TerrainOutput output;

    output.oPosition = input.Position.xyz;
    
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position;

    output.Position = ViewProjection(output.Position);
    output.wvpPosition = output.Position;
   
    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Color = input.Color;
    output.Uv = input.Uv;
    
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    return output;

}


DepthOutput VS_Depth_Terrain(VertexTerrain input)
{
    DepthOutput output;
     
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, ShadowView);
    output.Position = mul(output.Position, ShadowProjection);
    output.sPosition = output.Position;

    return output;
}
///////////////////////////////////////////////////////////////////////////////

struct BrushDesc
{
    float4 Color; //지역변수 맴버가된다.
    float3 Location;
    uint Type;
    uint Range;
};

cbuffer CB_TerrainBrush //struct로 선언하면 지역변수 cbuffer로 하면 전역으로 잡힌다.(쉐이더에서)
{
    BrushDesc TerrainBrush;
};


float3 GetBrushColor(float3 wPosition)
{
    [flatten]
    if (TerrainBrush.Type == 0)
        return float3(0, 0, 0);
     [flatten]
    if (TerrainBrush.Type == 1)
    {
        if ((wPosition.x > (TerrainBrush.Location.x - TerrainBrush.Range)) &&
            (wPosition.x < (TerrainBrush.Location.x + TerrainBrush.Range)) &&
            (wPosition.z > (TerrainBrush.Location.z - TerrainBrush.Range)) &&
            (wPosition.z < (TerrainBrush.Location.z + TerrainBrush.Range)))
        {
            return TerrainBrush.Color;
        }
    }
     [flatten]
    if (TerrainBrush.Type == 2)
    {
        float dx = wPosition.x - TerrainBrush.Location.x;
        float dz = wPosition.z - TerrainBrush.Location.z;

        float dist = sqrt(dx * dx + dz * dz);

           [flatten]
        if (dist <= TerrainBrush.Range)
            return TerrainBrush.Color;

    }
    return float3(0, 0, 0);
}

cbuffer CB_GridLine
{
    float4 GridLineColor;

    uint VisibleGridLine;
    float GridLineThickness;
    float GridLineSize; //몇칸당 하나?
};

//float3 GetLineColor(float3 wPosition)
//{
//    [flatten]
//    if(VisibleGridLine<1)
//        return float3(0, 0, 0);

//    float2 grid = wPosition.xz / GridLineSize;
//    //grid = abs(frac(grid - 0.5f) - 0.5f); //선굵기 균일?
//    grid = frac(grid);
//    float thick = GridLineThickness / GridLineSize;

//    [flatten]
//    if (grid.x < thick || grid.y < thick)
//        return GridLineColor.rgb;

//    return float3(0, 0, 0);
//}
float3 GetLineColor(float3 wPosition)
{
    [flatten]
    if (VisibleGridLine < 1)
        return float3(0, 0, 0);

    float2 grid = wPosition.xz / GridLineSize;
    float2 range = abs(frac(grid - 0.5f) - 0.5f); //중심점 잡기
    float2 speed = fwidth(grid); //비율
    float2 pixel = range / speed; //선분이 나올값.
    float thick = saturate(min(pixel.x, pixel.y) - GridLineThickness);

    //return GridLineColor.rgb * (1-thick);
    return lerp(GridLineColor.rgb, float3(0, 0, 0), thick);
}

float4 GetTerrainColor(float2 uv)
{
    //000_Header에 있으면 부르고 불러서 상관없음
    float4 base = BaseMap.Sample(LinearSampler, uv);
    float4 layer = LayerMap.Sample(LinearSampler, uv);
    float4 alpha = AlphaMap.Sample(LinearSampler, uv);
    float4 result = lerp(base, layer, alpha.r);

    return result;
}

float4 GetTerrainColor(float4 alpha, float2 uv)
{
    //000_Header에 있으면 부르고 불러서 상관없음
    float4 base = BaseMap.Sample(LinearSampler, uv);
     [flatten]
    if (any(base.a) == false)
    {
        float width = uv.x / Lod.TexelCellSpaceU + 0.5f;
        float height = uv.y / Lod.TexelCellSpaceV + 0.5f;
        uint size = Lod.TexScale * 8;
        float3 gray = float3(0.499f, 0.487f, 0.514f);
        float3 white = float3(0.899f, 0.887f, 0.914f);
        bool token = false;
        
        int w = width / size;
        int h = height / size;
        token = (w % 2 == 0) ? token : !token;
        token = (h % 2 == 0) ? token : !token;
        base = float4(lerp(gray, white, token), 1.0f);
    }
    
    
    float4 layer;
	//float alpha;
    float4 result;
    
    layer = LayerMaps[0].Sample(LinearSampler, uv);
	[flatten]
    if (any(layer))
        result = lerp(base, layer, alpha.r);

    layer = LayerMaps[1].Sample(LinearSampler, uv);
    [flatten]
    if (any(layer))
        result = lerp(result, layer, alpha.g);
    
    layer = LayerMaps[2].Sample(LinearSampler, uv);
	[flatten]
    if (any(layer))
        result = lerp(result, layer, alpha.b);

    return result;
}


float4 VS_AllLight_Terrain(TerrainOutput input)
{
    
    float4 diffuse = GetTerrainColor(input.Color, input.Uv);
    float3 gridColor = GetLineColor(input.wPosition);
    Material.Diffuse = diffuse;
     
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);
    
    return float4(MaterialToColor(result), 1) + float4(gridColor, 1);
}


float4 VS_Shadow_Terrain(TerrainOutput input, float4 color)
{
    input.sPosition.xyz /= input.sPosition.w;

    [flatten]
    if (input.sPosition.x < -1.0f || input.sPosition.x > 1.0f ||
        input.sPosition.y < -1.0f || input.sPosition.y > 1.0f ||
        input.sPosition.z < 0.0f || input.sPosition.z > 1.0f)
        return color;

    input.sPosition.x = input.sPosition.x * 0.5f + 0.5f;
    input.sPosition.y = -input.sPosition.y * 0.5f + 0.5f;
    input.sPosition.z -= ShadowBias;
    //return ShadowMap.Sample(LinearSampler, input.sPosition.xy);

    float depth = 0;
    float factor = 0;
   
    {
        float2 size = 1.0f / ShadowMapSize;
        float2 offsets[] =
        {
            float2(+2 * size.x, -2 * size.y), float2(+size.x, -2 * size.y), float2(0.0f, -2 * size.y), float2(-size.x, -2 * size.y), float2(-2 * size.x, -2 * size.y),
            float2(+2 * size.x, -size.y), float2(+size.x, -size.y), float2(0.0f, -size.y), float2(-size.x, -size.y), float2(-2 * size.x, -size.y),
            float2(+2 * size.x, 0.0f), float2(+size.x, 0.0f), float2(0.0f, 0.0f), float2(-size.x, 0.0f), float2(-2 * size.x, 0.0f),
            float2(+2 * size.x, +size.y), float2(+size.x, +size.y), float2(0.0f, +size.y), float2(-size.x, +size.y), float2(-2 * size.x, +size.y),
            float2(+2 * size.x, +2 * size.y), float2(+size.x, +2 * size.y), float2(0.0f, +2 * size.y), float2(-size.x, +2 * size.y), float2(-2 * size.x, +2 * size.y),
        };
        float weight[] =
        {
            1, 1, 2, 1, 1,
            1, 2, 4, 2, 1,
            2, 4, 8, 4, 2,
            1, 2, 4, 2, 1,
            1, 1, 2, 1, 1,
        };

        float sum = 0.0f;
        float totalweight = 0.0f;
        float2 uv = 0.0f;
        
        depth = input.sPosition.z;

        //[unroll(9)]
        for (int i = 0; i < 25; i++)
        {
            uv = input.sPosition.xy + offsets[i];
            totalweight += weight[i];
            sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, depth).r * weight[i];
        }

        factor = sum / totalweight;
    }
    factor = saturate(factor + depth);

    return float4(color.rgb * factor, 1);
    //return float4(factor, factor, factor, 1);
}

///////////////////////////////////////////////////////////////////////////////


float TessFactor(float3 position)
{
    //테셀 될 정점의 위치?
    //position.y = 0.0f;
    float3 view = ViewPosition();
    //view.y = 0.0f;
    float d = distance(position, view);
    float s = saturate((d - Lod.MinDistance) / (Lod.MaxDistance - Lod.MinDistance));

    s = int(s * 16.0f) / 16.0f;

    return lerp(Lod.MaxTessellation, Lod.MinTessellation, s);
}
//회전 x 상태
bool AabbBehindPlaneText(float3 center, float3 extents, float4 plane)
{
    float3 n = normalize(plane.xyz);
    //float3 n = abs(plane.xyz);
    float r = dot(extents, n);
    //float r = dot(extents, n);
    float s = dot(float4(center, 1), plane);

    return (s + r) < 0.0f;
}

bool ContainFrustumCube(float3 center, float3 extents)
{
    //[roll(6)]
    for (int i = 0; i < 6; i++)
    {
        [flatten]
        if (AabbBehindPlaneText(center, extents, Planes[i]))
            return true;
    }
    return false;
}