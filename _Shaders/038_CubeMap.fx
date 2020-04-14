#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"


float4 PS(MeshOutput input) : SV_Target0
{
    return VS_Shadow(input, VS_AllLight(input));
}

float2 RecflectAmount;

TextureCube CubeMap;
float4 PS_Cube(MeshOutput input) : SV_Target0
{
    float4 color = VS_Shadow(input, VS_AllLight(input));
    
    float3 eye = normalize(input.wPosition -ViewPosition());
    float3 r = reflect(eye, normalize(input.Normal));
    //float4 color = VS_AllLight(input);


    color *= (RecflectAmount.x + CubeMap.Sample(LinearSampler, r) * RecflectAmount.y);

    return color;
   // return CubeMap.Sample(LinearSampler, r);
}


RasterizerState RS
{
    CullMode = Front;
};
technique11 T0
{
    P_RS_VP(P0,RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1,RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2,RS, VS_Depth_Animation, PS_Depth)

    P_VP(P3, VS_Mesh, PS)
    P_VP(P4, VS_Model, PS)
    P_VP(P5, VS_Animation, PS)

    P_VP(P6, VS_Mesh, PS_Cube)
    P_VP(P7, VS_Model, PS_Cube)
    P_VP(P8, VS_Animation, PS_Cube)
}