
Texture2D BaseMap;
Texture2D AlphaMap;

Texture2D LayerMaps[3];

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
    float GridLineSize; 
};
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
float4 GetTerrainLodColor(float4 alpha, float2 uv)
{
    //000_Header에 있으면 부르고 불러서 상관없음
    float4 base = BaseMap.Sample(LinearSampler, uv);
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
float4 Planes[6];

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
    float inside[2] : SV_InsideTessFactor;
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


VertexOutput_Lod VS(Vertexinput_Lod input)
{
    VertexOutput_Lod output;
    output.Position = WorldPosition(input.Position);
    output.Uv = input.Uv;
    output.BoundY = input.BoundY;
    
    return output;
}
uint UseLOD;
//                                                             ↓ controll_point
ConstantHullOutput_Lod HS_Constant(InputPatch<VertexOutput_Lod, 4> input)
{
    float minY = input[0].BoundY.x;
    float maxY = input[0].BoundY.y;

    float3 vMin = float3(input[0].Position.x, minY, input[0].Position.z);
    float3 vMax = float3(input[3].Position.x, maxY, input[3].Position.z);
    float3 boxCenter = (vMin + vMax) * 0.5f;
    float3 boxExtents = (vMin - vMax) * 0.5f;

    ConstantHullOutput_Lod output;
    if (UseLOD < 1)
    {
        output.Edge[0] = output.Edge[1] =
        output.Edge[2] = output.Edge[3] =
        output.inside[0] = output.inside[1] = Lod.MaxTessellation;
        return output;

    }
   

    float3 e0 = (input[0].Position + input[2].Position).xyz * 0.5f;
    float3 e1 = (input[0].Position + input[1].Position).xyz * 0.5f;
    float3 e2 = (input[1].Position + input[3].Position).xyz * 0.5f;
    float3 e3 = (input[2].Position + input[3].Position).xyz * 0.5f;

    output.Edge[0] = TessFactor(e0);
    output.Edge[1] = TessFactor(e1);
    output.Edge[2] = TessFactor(e2);
    output.Edge[3] = TessFactor(e3);

    float3 c = (input[0].Position + input[1].Position + input[2].Position + input[3].Position).xyz;
    c *= 0.25f;
    output.inside[0] = TessFactor(c);
    output.inside[1] = TessFactor(c);

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64)] //최대 정해주는게 빠르다?
HullOutput_Lod HS(InputPatch<VertexOutput_Lod, 4> input, uint pointID : SV_OutputControlPointID)
{
    HullOutput_Lod output;
    output.Position = input[pointID].Position;
    output.Uv = input[pointID].Uv;
    return output;
}

[domain("quad")]
DomainOutput_Lod DS(ConstantHullOutput_Lod input, const OutputPatch<HullOutput_Lod, 4> patch, float2 uv : SV_DomainLocation)
{
    DomainOutput_Lod output;
    float3 p0 = lerp(patch[0].Position, patch[1].Position, uv.x).xyz;
    float3 p1 = lerp(patch[2].Position, patch[3].Position, uv.x).xyz;

    float3 position = lerp(p0, p1, uv.y);
    output.wPosition = position;

    float2 uv0 = lerp(patch[0].Uv, patch[1].Uv, uv.x);
    float2 uv1 = lerp(patch[2].Uv, patch[3].Uv, uv.x);

    output.Uv = lerp(uv0, uv1, uv.y);
    //노멀 구하기
    float2 left = output.Uv + float2(-Lod.TexelCellSpaceU, -Lod.TexelCellSpaceV);
    float2 right = output.Uv + float2(Lod.TexelCellSpaceU, Lod.TexelCellSpaceV);
    float2 top = output.Uv + float2(0.0f, -Lod.TexelCellSpaceV);
    float2 bottom = output.Uv + float2(0.0f, Lod.TexelCellSpaceV);

    //구한 uv를 통해 heightmap에서 해당 위치의 높이를 구한다.
    //노멀,탄젠트를 구할수 있다.
    float leftY = AlphaMap.SampleLevel(LinearSampler, left, 0).a * Lod.TerrainHeightRatio;
    float rightY = AlphaMap.SampleLevel(LinearSampler, right, 0).a * Lod.TerrainHeightRatio;
    float topY = AlphaMap.SampleLevel(LinearSampler, top, 0).a * Lod.TerrainHeightRatio;
    float bottomY = AlphaMap.SampleLevel(LinearSampler, bottom, 0).a * Lod.TerrainHeightRatio;

    //탄젠트는 x축 기울기와 같다
    output.Tangent = normalize(float3(Lod.WorldCellSpace * 2.0f, rightY - leftY, 0.0f));
    //uv가 뒤집혀 있음에 주의
    float3 biTangent = normalize(float3(0.0f, bottomY - topY, Lod.WorldCellSpace * -2.0f));
    //높이차에 의한 노멀
    output.Normal = cross(output.Tangent, biTangent);
    
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(output.wPosition, ViewPosition().xyz) - MipInterval) / MipInterval, 0.0f, 6.0f);
    
    //맵에 따른 표면 노멀
    float3 normal = NormalMap.SampleLevel(LinearSampler, output.Uv, mipLevel);
    //normal = (output.Normal * (normal * 2.0f - 1.0f));
    //
    //samplelevel mipmap 제거?
    //                      해당 위치에 대한 픽셀을 가져올 수 있음
    //float displacement = DisplaceMentMap.SampleLevel(LinearSampler, output.Uv, 0).r;

    output.wPosition.y += AlphaMap.SampleLevel(LinearSampler, output.Uv, 0).a * Lod.TerrainHeightRatio;
    //output.wPosition.xyz += displacement * normal;
    
    output.Position = ViewProjection(float4(output.wPosition, 1));

    output.TiledUv = output.Uv * Lod.TexScale;

    
    output.Clip.x = dot(float4(output.wPosition, 1), Planes[0]);
    output.Clip.y = dot(float4(output.wPosition, 1), Planes[1]);
    output.Clip.z = dot(float4(output.wPosition, 1), Planes[2]);
    output.Clip.w = 0.0f;
    output.Clip2.x = dot(float4(output.wPosition, 1), Planes[3]);
    output.Clip2.y = dot(float4(output.wPosition, 1), Planes[4]);
    output.Clip2.z = dot(float4(output.wPosition, 1), Planes[5]);
    output.Clip2.w = 0.0f;
    return output;
}
