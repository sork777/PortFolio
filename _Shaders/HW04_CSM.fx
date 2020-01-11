#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

Texture2DArray CSMTexture;
SamplerComparisonState CSMSampler;

struct DirShadow
{
    float4x4 ToShadowSpace;
    float4 ToCascadeOffsetX;
    float4 ToCascadeOffsetY;
    float4 ToCascadeScale;
};
cbuffer CB_DirShadow
{
    DirShadow DirCSM;
};

float CascadedShadow(float3 position)
{
	// Transform the world position to shadow space
    float4 posShadowSpace = mul(float4(position, 1.0), DirCSM.ToShadowSpace);

	// Transform the shadow space position into each cascade position
    float4 posCascadeSpaceX = (DirCSM.ToCascadeOffsetX + posShadowSpace.xxxx) * DirCSM.ToCascadeScale;
    float4 posCascadeSpaceY = (DirCSM.ToCascadeOffsetY + posShadowSpace.yyyy) * DirCSM.ToCascadeScale;

	// Check which cascade we are in
    float4 inCascadeX = abs(posCascadeSpaceX) <= 1.0;
    float4 inCascadeY = abs(posCascadeSpaceY) <= 1.0;
    float4 inCascade = inCascadeX * inCascadeY;

	// Prepare a mask for the highest quality cascade the position is in
    float4 bestCascadeMask = inCascade;
    bestCascadeMask.yzw = (1.0 - bestCascadeMask.x) * bestCascadeMask.yzw;
    bestCascadeMask.zw = (1.0 - bestCascadeMask.y) * bestCascadeMask.zw;
    bestCascadeMask.w = (1.0 - bestCascadeMask.z) * bestCascadeMask.w;
    float bestCascade = dot(bestCascadeMask, float4(0.0, 1.0, 2.0, 3.0));

	// Pick the position in the selected cascade
    float3 UVD;
    UVD.x = dot(posCascadeSpaceX, bestCascadeMask);
    UVD.y = dot(posCascadeSpaceY, bestCascadeMask);
    UVD.z = posShadowSpace.z;

	// Convert to shadow map UV values
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;

	// Compute the hardware PCF value
    float shadow = CSMTexture.SampleCmpLevelZero(CSMSampler, float3(UVD.xy, bestCascade), UVD.z);
	
	// set the shadow to one (fully lit) for positions with no cascade coverage
    shadow = saturate(shadow + 1.0 - any(bestCascadeMask));
	
    return shadow;
}


struct ShadowCSM
{
    matrix ViewProjection[3];
    float2 ShadowMapSize;
    float ShadowBias;
};
cbuffer CB_CSM
{
    ShadowCSM cascadeShadow;
};

float4 ShadowGenVS(float4 Pos : POSITION) : SV_Position
{
    return Pos;
}

struct GS_CSM_Output
{
    float4 Position : SV_Position0;
    uint TargetIndex : SV_RenderTargetArrayIndex; //ArraySize의 번호
};

[maxvertexcount(9)]
void GS_PreRender(triangle float4 input[3] : SV_Position, inout TriangleStream<GS_CSM_Output> stream)
//void GS_PreRender(triangle MeshOutput input[3], inout TriangleStream<GS_CSM_Output> stream)
{
    //[unroll(8)]
    for (int i = 0; i < 3; i++)
    {
        GS_CSM_Output output;
        output.TargetIndex = i;
        for (int vertex = 0; vertex < 3; vertex++)
        {
            output.Position = mul(input[vertex], cascadeShadow.ViewProjection[i]);
            //output.Position = mul(input[vertex].Position, cascadeShadow.ViewProjection[i]);
            stream.Append(output);
        }
        stream.RestartStrip();
    }

}
float3 CalcDirectional(float3 position, float3 normal, float4 SpecularColor, float4 DiffuseColor)
{

	// Phong diffuse
    float NDotL = dot(-GlobalLight.Direction, normal);
    float3 finalColor = DiffuseColor.rgb * NDotL;
   
	// Blinn specular
    float3 ToEye = ViewPosition() - position;
    ToEye = normalize(ToEye);
    //float3 HalfWay = normalize(ToEye - GlobalLight.Direction);
    //float NDotH = saturate(dot(HalfWay, normal));
    float3 R = normalize(reflect(GlobalLight.Direction, normal)); /* 들어오는 방향 */
    float RdotE = saturate(dot(R, ToEye));

    finalColor += DiffuseColor.rgb * pow(RdotE, SpecularColor.a) * SpecularColor.rgb;
    float shadowAtt = CascadedShadow(position);
    //return float3(shadowAtt, 0, 0);
    return finalColor * DiffuseColor.rgb * shadowAtt;
}

/////////////////////////////////////////////////////////////////////////////
float3 CalNormal(float2 uv, float3 normal, float3 tangent, SamplerState sample)
{
    float4 map = NormalMap.Sample(sample, uv);
    
    [flatten]
    if (any(map) == false)
        return normal;

    //탄젠트 공간
    float3 N = normalize(normal); //Z
    float3 T = normalize(tangent - dot(tangent, N) * N); //X
    float3 B = cross(N, T); //Y
    float3x3 TBN = float3x3(T, B, N);

    //이미지로 부터 노멀벡터 가져오기
    float3 coord = map.rgb * 2.0f - 1.0f;

    return mul(coord, TBN);
}

float3 CalNormal(float2 uv, float3 normal, float3 tangent)
{
    return CalNormal(uv, normal, tangent, LinearSampler);
}

float4 PS(MeshOutput input) : SV_Target0
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);
    float3 normal = CalNormal(input.Uv, input.Normal, input.Tangent);
    Texture(Material.Specular, SpecularMap, input.Uv);
    float4 DiffuseColor = DiffuseMap.Sample(LinearSampler, input.Uv);
    float4 SpecularColor = SpecularMap.Sample(LinearSampler, input.Uv);
    
    float3 color = CalcDirectional(input.Position.xyz, normal, SpecularColor, DiffuseColor);
   
    return float4(color.rgb, 1);
    
}

technique11 T0
{
    pass P0
    {
        SetRasterizerState(RS);
        //SetVertexShader(CompileShader(vs_5_0, VS_Mesh_GS()));
        SetVertexShader(CompileShader(vs_5_0, ShadowGenVS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
        //SetPixelShader(CompileShader(ps_5_0, PS_PreRender()));
    }
    pass P1
    {
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, ShadowGenVS()));
        //SetVertexShader(CompileShader(vs_5_0, VS_Model_GS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
    }
    pass P2
    {
        SetRasterizerState(RS);
        SetVertexShader(CompileShader(vs_5_0, ShadowGenVS()));
        //SetVertexShader(CompileShader(vs_5_0, VS_Animation_GS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
    }

    P_VP(P3, VS_Mesh, PS)
    P_VP(P4, VS_Model, PS)
    P_VP(P5, VS_Animation, PS)
}