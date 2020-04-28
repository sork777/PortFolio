#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Terrain.fx"


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
    [flatten]
    if (ContainFrustumCube(boxCenter, boxExtents))
    {
        output.Edge[0] = 0;
        output.Edge[1] = 0;
        output.Edge[2] = 0;
        output.Edge[3] = 0;

        output.Inside[0] = 0;
        output.Inside[1] = 0;

        return output;
    }
    [flatten]
    if (UseLOD < 1)
    {
        output.Edge[0] = output.Edge[1] =
        output.Edge[2] = output.Edge[3] =
        output.Inside[0] = output.Inside[1] = Lod.MaxTessellation;
        return output;

    }
   

    float3 e0 = (input[0].Position + input[2].Position).xyz * 0.5f;
    float3 e1 = (input[0].Position + input[1].Position).xyz*0.5f;
    float3 e2 = (input[1].Position + input[3].Position).xyz*0.5f;
    float3 e3 = (input[2].Position + input[3].Position).xyz*0.5f;

    output.Edge[0] = TessFactor(e0);
    output.Edge[1] = TessFactor(e1);
    output.Edge[2] = TessFactor(e2);
    output.Edge[3] = TessFactor(e3);

    float3 c = (input[0].Position + input[1].Position + input[2].Position + input[3].Position).xyz;
    c *= 0.25f;
    output.Inside[0] = TessFactor(c);
    output.Inside[1] = TessFactor(c);

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64)]     //최대 정해주는게 빠르다?
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
    float h = length(NormalMap.SampleLevel(LinearSampler, output.Uv, mipLevel));
    //
    //samplelevel mipmap 제거?
    //                      해당 위치에 대한 픽셀을 가져올 수 있음
    output.wPosition.xyz += (4.5f * (h)) * normalize(output.Normal);
    output.wPosition.y += AlphaMap.SampleLevel(LinearSampler, output.Uv, 0).a* Lod.TerrainHeightRatio;
    //output.wPosition.xyz += displacement*normal;
    
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

float4 PS(DomainOutput_Lod input) : SV_Target0
{
    float4 alpha = AlphaMap.Sample(LinearSampler, input.Uv);
    float4 diffuse = GetTerrainColor(alpha, input.Uv);
    float3 gridColor = GetLineColor(input.wPosition);
    float3 normal = input.Normal;
    float3 brushColor = GetBrushColor(input.wPosition);

    Material.Diffuse = diffuse;
     
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, normal, input.wPosition);
    AddMaterial(result, output);
    
    //return AlphaMap.SampleLevel(LinearSampler, input.Uv, 0);
    return float4(MaterialToColor(result), 1) + float4(gridColor, 1) + float4(brushColor, 1);
    //return float4(1, 0, 0, 1);
}
RasterizerState RS
{
    Fillmode = Wireframe;
};

struct PS_Output
{
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Spec : SV_Target2;
    float4 Depth : SV_Target3;
    float4 Emissive : SV_Target4;
};

PS_Output PS_Seperate(DomainOutput_Lod input)
{
    PS_Output output;
    

    float4 alpha = AlphaMap.Sample(LinearSampler, input.Uv);
    float4 diffuse = GetTerrainColor(alpha, input.Uv*3.0f);
    float3 gridColor = GetLineColor(input.wPosition);
    float3 brushColor = GetBrushColor(input.wPosition);
    float3 normal = normalize(input.Normal);
    float depth = input.Position.z / input.Position.w;
    Texture(Material.Specular, SpecularMap, input.Uv);
    
    diffuse = diffuse + float4(gridColor, 1) + float4(brushColor, 1);
    float4 SpecularColor = Material.Specular;

    output.Color = float4(diffuse.rgb, SpecularColor.a);
    output.Normal = float4(normal * 0.5f + 0.5f, 0);
    output.Spec = float4(SpecularColor.rgb, 0);
    output.Depth = float4(depth, depth, depth, 1);
    output.Emissive = Material.Emissive;
    return output;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }

    pass P1
    {
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
technique11 T1_Deffered
{
    pass P0
    {
        SetDepthStencilState(DepthMarkDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Seperate()));
    }
    pass P1
    {
        SetRasterizerState(RS);
        SetDepthStencilState(DepthMarkDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS_Seperate()));
    }
}