#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
#include "000_Terrain.fx"


float4 PS(MeshOutput input) : SV_Target0
{
    float len = length(input.wPosition - ViewInverse._41_42_43);
    float4 color = VS_Shadow(input, VS_AllLight(input));
  
    return float4(color.rgb, 1);
    
}

float4 PS_Terrain(TerrainOutput input) : SV_Target0
{
    float4 color = VS_AllLight_Terrain(input);
    //color = VS_Shadow(input,color);
    

    return float4(color.rgb, 1);
    
}
RasterizerState RS
{
    CullMode = Front;
};
technique11 T0
{
    P_RS_VP(P0,RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1,RS, VS_Depth_MeshInst, PS_Depth)
    P_RS_VP(P2,RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P3,RS, VS_Depth_Animation, PS_Depth)
    P_RS_VP(P4, RS, VS_Depth_Terrain, PS_Depth)

    P_VP(P5, VS_Mesh, PS)
    P_VP(P6, VS_MeshInst, PS)
    P_VP(P7, VS_Model, PS)
    P_VP(P8, VS_Animation, PS)
    P_VP(P9, VS_Terrain, PS_Terrain)
}