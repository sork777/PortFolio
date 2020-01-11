#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

matrix CubeViews[6];
matrix CubeProjection;


[maxvertexcount(18)]
void GS_PreRender(triangle MeshOutput input[3], inout TriangleStream<GeometryOutput> stream)
{
    int vertex = 0;
    GeometryOutput output;

    //[unroll(8)]
    for (int i = 0; i < 6;i++)
    {
        output.TargetIndex = i;
        for (vertex = 0; vertex < 3;vertex++)
        {
            output.Position = mul(input[vertex].Position, CubeViews[i]);
            output.Position = mul(output.Position, CubeProjection);
            output.wPosition = input[vertex].wPosition;
            output.wvpPosition = input[vertex].wvpPosition;
            output.oPosition = input[vertex].oPosition;
            output.sPosition = input[vertex].sPosition;
            output.Uv = input[vertex].Uv;
            output.Normal = input[vertex].Normal;
            output.Tangent = input[vertex].Tangent;
            stream.Append(output);
        }
        stream.RestartStrip();
    }

}

float4 PS_PreRender(GeometryOutput input) : SV_Target0
{
    return VS_Shadow(input, VS_AllLight(input));
}

float4 PS(MeshOutput input) : SV_Target0
{
    //return VS_AllLight(input);
    //DiffuseMap.Sample(LinearSampler, input.Uv);
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

    return CubeMap.Sample(LinearSampler, r);
    //return color;
}


RasterizerState RS
{
    CullMode = Front;
};
technique11 T0
{
    //Cube - PreRender
    P_VGP(P0, VS_Mesh_GS, GS_PreRender,PS_PreRender)
    P_VGP(P1, VS_Model_GS, GS_PreRender, PS_PreRender)
    P_VGP(P2, VS_Animation_GS, GS_PreRender, PS_PreRender)


    //Depth
    P_RS_VP(P3,RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P4,RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P5,RS, VS_Depth_Animation, PS_Depth)

    //Render
    P_VP(P6, VS_Mesh, PS)
    P_VP(P7, VS_Model, PS)
    P_VP(P8, VS_Animation, PS)

    //CubeMap_Render
    P_VP(P9, VS_Mesh, PS_Cube)
    P_VP(P10, VS_Model, PS_Cube)
    P_VP(P11, VS_Animation, PS_Cube)
}