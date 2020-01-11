#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

cbuffer CB_Sky
{
    float4 Center;
    float4 Apex;

    float Height;
};

float4 PS(MeshOutput input) : SV_Target0
{
    //lerp의 t값은 0~1이어야 한다.?
    //return lerp(Center, Apex, input.oPosition.y*2);
    //높이 보정
    return lerp(Center, Apex, input.oPosition.y * Height);
}

float4 PS_CubeMap(MeshOutput input) : SV_Target0
{
    return SkyCubeMap.Sample(LinearSampler, input.oPosition);
}

///////////////////////////////////////////////////////////////////////////////

RasterizerState RS
{
    FrontCounterClockWise = true;
};

RasterizerState RS2
{
    Fillmode = Wireframe;

    FrontCounterClockWise = true;
};
///////////////////////////////////////////////////////////////////////////////

DepthStencilState DS
{
    DepthEnable = false;
};

technique11 T0
{
    P_RS_DSS_VP(P0, RS, DS, VS_Mesh, PS)
    P_RS_DSS_VP(P1, RS2, DS, VS_Mesh, PS)
    P_RS_DSS_VP(P2, RS, DS, VS_Mesh, PS_CubeMap)
}