#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"
#define MAX_DISTANCE 200
#define MIN_DISTANCE 1
#define MAX_TESS 32
#define MIN_TESS 1

float DisplacementFactor;

float TessFactor(float3 position)
{
    //테셀 될 정점의 위치?
    //position.y = 0.0f;
    float3 view = ViewPosition();
    //view.y = 0.0f;
    float d = distance(position, view);
    float s = saturate((d - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE));

    s = int(s * 4.0f) / 4.0f;

    return lerp(MAX_TESS, MIN_TESS,s);
}

struct ConstantHullOutput
{
    float Edge[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

ConstantHullOutput HS_Constant(InputPatch<MeshTessOutput, 4> input)
{
    ConstantHullOutput output;
       
    //Quad
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

struct HullOutput
{
    float4 Position : Position0;
    float2 Uv : Uv0;
     
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_Constant")]
[maxtessfactor(64)]     
HullOutput HS(InputPatch<MeshTessOutput, 4> input, uint pointID : SV_OutputControlPointID)
{
    HullOutput output;
    output.Position = input[pointID].Position;
    output.Uv = input[pointID].Uv;
    output.Normal = input[pointID].Normal;
    output.Tangent = input[pointID].Tangent;
    
    return output;
}

struct DomainOutput
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float2 Uv : Uv0;
     
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
};

[domain("quad")]
DomainOutput DS(ConstantHullOutput input, const OutputPatch<HullOutput, 4> patch, float2 uv : SV_DomainLocation)
{
    DomainOutput output;
    //Quad
    float3 p0 = lerp(patch[0].Position, patch[1].Position, uv.x).xyz;
    float3 p1 = lerp(patch[2].Position, patch[3].Position, uv.x).xyz;

    float3 position = lerp(p0, p1, uv.y);
    output.wPosition = position;

    float2 uv0 = lerp(patch[0].Uv, patch[1].Uv, uv.x);
    float2 uv1 = lerp(patch[2].Uv, patch[3].Uv, uv.x);
    output.Uv = lerp(uv0, uv1, uv.y);
    
    float3 n0 = lerp(patch[0].Normal, patch[1].Normal, uv.x).xyz;
    float3 n1 = lerp(patch[2].Normal, patch[3].Normal, uv.x).xyz;
    output.Normal = lerp(n0, n1, uv.y);
    
    float3 t0 = lerp(patch[0].Tangent, patch[1].Tangent, uv.x).xyz;
    float3 t1 = lerp(patch[2].Tangent, patch[3].Tangent, uv.x).xyz;
    output.Tangent = lerp(t0, t1, uv.y);
    
    NormalMapping(output.Uv, output.Normal, output.Tangent);
    
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(output.wPosition, ViewPosition().xyz) - MipInterval) / MipInterval, 0.0f, 6.0f);
    float3 height = HeightMap.SampleLevel(LinearSampler, output.Uv, mipLevel);
    float h = length(height);
    
    //변위값 x 해당 위치의 법선 방향 x 해당 위치에서의 변동위치 만큼 원래 위치에서 더함
    output.wPosition.xyz += DisplacementFactor * (normalize(output.Normal) * (h - 0.5f));
    output.Position = ViewProjection(float4(output.wPosition, 1));
    return output;
}

float4 PS(MeshOutput input) : SV_Target0
{
    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    Material.Diffuse *= diffuse;
    
    NormalMapping(input.Uv, input.Normal, input.Tangent);
    float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);
    Material.Diffuse *= NdotL; 
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    float3 color = MaterialToColor(result);
    
    return float4(color, 1);
}

float4 PS_PBRTest(MeshOutput input) : SV_Target0
{
    return CalcNormaltoPBR(input);
}


float4 PS_Tess_PBRTest(DomainOutput input) : SV_Target0
{
    MeshOutput output;
    output.Position =input.Position;
    output.wPosition =input.wPosition;
    output.Uv = input.Uv;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    output.oPosition = input.wPosition;
    output.wvpPosition = input.Position;
    output.sPosition = input.Position;
    output.Clip = 0;
    output.ID = 0;
    return CalcNormaltoPBR(output);
}

float4 WirePS(MeshOutput input) : SV_Target0
{
    return float4(0, 0, 0, 1);
}

float4 WireTessPS(DomainOutput input) : SV_Target0
{
    return float4(0, 0, 0, 1);
}

technique11 T0
{
    P_VP(P0, VS_Mesh, PS)
    P_VP(P1, VS_Model, PS)
    P_VP(P2, VS_Animation, PS)
}

technique11 T_PBR
{
    P_VP(P0, VS_Mesh, PS_PBRTest)
    P_VTP(P1, VS_Tess_Mesh, HS, DS, PS_Tess_PBRTest)
}
technique11 TWireFrame
{
    P_RS_VP(P0, WireFrameRS, VS_Mesh, WirePS)
    P_RS_VTP(P1, WireFrameRS, VS_Tess_Mesh, HS, DS, WireTessPS)
}
