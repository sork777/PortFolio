#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
#include "000_Terrain.fx"

cbuffer CB_Projector
{
    matrix View2;
    matrix Projection2;
    float4 ProjectorColor;

    int draw;
};

Texture2D ProjectorMap;

void ProjectorPosition(inout float4 wvp, float4 position)
{
    //wPosition은 float3라 안됨
    wvp = WorldPosition(position);
    wvp = mul(wvp, View2);
    wvp = mul(wvp, Projection2);
}

/* 메시용 */
MeshOutput VS_Projector_Mesh(VertexMesh input)
{
    MeshOutput output = VS_Mesh(input);
    /* 카메라의 시야가 아니라 프로젝터의 시점으로 만들기 위함 */
    ProjectorPosition(output.wvpPosition, input.Position);
    return output;
}

/* 모델 용 */
MeshOutput VS_Projector_Model(VertexModel input)
{
    MeshOutput output = VS_Model(input);
    ProjectorPosition(output.wvpPosition, input.Position);
    return output;
}

/* 애니메이션 용 */
MeshOutput VS_Projector_Animation(VertexModel input)
{
    MeshOutput output = VS_Animation(input);
    ProjectorPosition(output.wvpPosition, input.Position);
    return output;
}
/* 터레인 */
TerrainOutput VS_Projector_Terrain(VertexTerrain input)
{
    TerrainOutput output = VS_Terrain(input);
    ProjectorPosition(output.wvpPosition, input.Position);
    return output;
}


float4 PS(MeshOutput input) : SV_Target0
{
    float4 color = VS_AllLight(input);

    float2 uv = 0;
    /* 화면에 나갈 비율 */
    uv.x = input.wvpPosition.x / input.wvpPosition.w * 0.5f + 0.5f;
    uv.y = -input.wvpPosition.y / input.wvpPosition.w * 0.5f + 0.5f;
    
    /* 프로젝터의 범위 벗어나는 애들 빼려고 */
    [flatten]
    if (saturate(uv.x) == uv.x && saturate(uv.y) == uv.y)
    {
        float4 map = ProjectorMap.Sample(LinearSampler, uv);
        map *= ProjectorColor;
        color = lerp(color, map, map.a);
    }
    
    return color;
}

float4 PS_Terrain(TerrainOutput input) : SV_Target0
{
    float4 color = VS_AllLight_Terrain(input);

    float2 uv = 0;
    /* 화면에 나갈 비율 */
    uv.x = input.wvpPosition.x / input.wvpPosition.w * 0.5f + 0.5f;
    uv.y = -input.wvpPosition.y / input.wvpPosition.w * 0.5f + 0.5f;
    
    /* 프로젝터의 범위 벗어나는 애들 빼려고 */
    [flatten]
    if (saturate(uv.x) == uv.x && saturate(uv.y) == uv.y && draw>=0)
    {
        float4 map = ProjectorMap.Sample(LinearSampler, uv);
        map *= ProjectorColor;
        color = lerp(color, map, map.a);
    }
    
    return color;
}

RasterizerState RS
{
    Fillmode = Wireframe;
};
technique11 T0
{
    P_VP(P0, VS_Projector_Mesh, PS)
    P_VP(P1, VS_Projector_Model, PS)
    P_VP(P2, VS_Projector_Animation, PS)
    P_VP(P3, VS_Projector_Terrain, PS_Terrain)
}