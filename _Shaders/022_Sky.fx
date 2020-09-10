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
    //lerp�� t���� 0~1�̾�� �Ѵ�.?
    //return lerp(Center, Apex, input.oPosition.y*2);
    //���� ����
    return lerp(Center, Apex, input.oPosition.y * Height);
}
//���� Global���� �ν� �ȵ�
float3 glDir;
float4 PS_CubeMap(MeshOutput input) : SV_Target0
{
    float4 color = SkyCubeMap.Sample(LinearSampler, input.oPosition);
    float U = saturate(dot(-(glDir), float3(0, 1, 0)));
    U = clamp(U, 0.3f, 1.0f);
    
    return color*U;
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