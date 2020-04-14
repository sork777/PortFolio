#include "HW00_common.fx"
#include "HW00_TerrainLod.fx"
#include "HW00_Light.fx"
#include "HW00_DeferredFunc.fx"



float4 PS(MeshOutput input) : SV_Target0
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);

    NormalMapping(input.Uv, input.Normal, input.Tangent);

    Texture(Material.Specular, SpecularMap, input.Uv);

    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);

    return diffuse;
}
/////////////////////////////////////////////////////////////////////////////
// Deferred Packing
/////////////////////////////////////////////////////////////////////////////

struct PS_Output
{
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Spec : SV_Target2;
    float4 Depth : SV_Target3;
    float4 Emissive : SV_Target4;
};

PS_Output PS_Seperate(MeshOutput input)
{
    PS_Output output;
    
    Texture(Material.Diffuse, DiffuseMap, input.Uv);
    float3 normal = CalNormal(input.Uv, input.Normal, input.Tangent);
    Texture(Material.Specular, SpecularMap, input.Uv);
    
    //NormalMapping(input.Uv, input.Normal, input.Tangent, BasicSampler);
    normal = normalize(normal);
    
    float4 DiffuseColor = Material.Diffuse;//    DiffuseMap.Sample(LinearSampler, input.Uv);
    float4 SpecularColor = Material.Specular; //SpecularMap.Sample(LinearSampler, input.Uv);
    //DiffuseColor *= DiffuseColor;
    
    float depth = input.wvpPosition.z / input.wvpPosition.w;

    output.Color = float4(DiffuseColor.rgb, SpecularColor.a);
    output.Normal = float4(normal * 0.5f + 0.5f, 0);
    output.Spec = float4(SpecularColor.rgb, 0);
    output.Depth = float4(depth, depth, depth, 1);
    output.Emissive = Material.Emissive;

    return output;
}

PS_Output PS_SeperateTerrainLod(DomainOutput_Lod input)
{
    PS_Output output;

    float4 alpha = AlphaMap.Sample(LinearSampler, input.Uv);
    float4 diffuse = GetTerrainLodColor(alpha, input.Uv*3.0f);
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
/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////


struct VS_OUTPUT
{
    float4 Position : SV_Position; // vertex position 
    float2 cpPos : TEXCOORD0;
    float2 Uv : TEXCOORD1;
};

VS_OUTPUT DirLightVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT Output;

    Output.Position = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    Output.cpPos = Output.Position.xy;
    Output.Uv = arrUV[VertexID].xy;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shaders - Directional
/////////////////////////////////////////////////////////////////////////////
uint UseAO;
float4 DirLightPS(VS_OUTPUT input) : SV_TARGET
{
	// Unpack the GBuffer
    SURFACE_DATA gbd = UnpackGBuffer_Loc(input.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 wPos = CalcWorldPos(input.cpPos, gbd.LinearDepth);
        
    float ao=AOTexture.Sample(LinearSampler, input.Uv);
    ao = lerp(ao, 1, UseAO);

    //Calculate the ambient color
    float3 finalColor = 0;//    CalcAmbient(mat.normal, mat.diffuseColor.rgb);
    
    // Calculate the directional light
    finalColor += CalcDirectional(wPos, mat);
    finalColor *= ao;
    if (gbd.LinearDepth>800)
        finalColor = AtmosphereMap.Sample(LinearSampler, input.Uv);

    return float4(finalColor, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader - Point
/////////////////////////////////////////////////////////////////////////////

float4 PointLightPS(DS_OUTPUT input) : SV_TARGET0
{
	// Unpack the GBuffer
   
    SURFACE_DATA gbd = UnpackGBuffer_Loc(input.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 position = CalcWorldPos(input.cpPos, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcPoint(position, mat);
    return float4(finalColor, 1.0);
}


/////////////////////////////////////////////////////////////////////////////
// Pixel shader - Spot
/////////////////////////////////////////////////////////////////////////////

float4 SpotLightPS(DS_SPOT_OUTPUT input) : SV_TARGET0
{
	// Unpack the GBuffer
    SURFACE_DATA gbd = UnpackGBuffer_Loc(input.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 position = CalcWorldPos(input.PositionXYW.xy / input.PositionXYW.z, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcSpot(position, mat);

	// Return the final color
    //return float4(1,0,0, 1.0);
    return float4(finalColor, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader - Capsule
/////////////////////////////////////////////////////////////////////////////


float4 CapsuleLightPS(DS_CapsuleOutput In) : SV_TARGET0
{
	// Unpack the GBuffer
    SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Position.xy);
	
	// Convert the data into the material structure
    DeferredMaterial mat;
    MaterialFromGBuffer(gbd, mat);

	// Reconstruct the world position
    float3 position = CalcWorldPos(In.cpPos, gbd.LinearDepth);

	// Calculate the light contribution
    float3 finalColor = CalcCapsule(position, mat);

	// Return the final color
    return float4(finalColor, 1.0);
}

technique11 T0
{
    ////////////////////////
    /* Render for GPacker */
    ////////////////////////
    P_DSS_Ref_VP(P0, DepthMarkDSS, 1, VS_Mesh, PS_Seperate)
    P_DSS_Ref_VP(P1, DepthMarkDSS, 1, VS_Model, PS_Seperate)
    P_DSS_Ref_VP(P2, DepthMarkDSS, 1, VS_Animation, PS_Seperate)
    
    pass P3
    {
        SetDepthStencilState(DepthMarkDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS_SeperateTerrainLod()));
    }
}
//1
technique11 T1_DirLight
{
    ///////////////////////
    /* Directional Light */
    ///////////////////////
    P_DSS_Ref_VP(P0, DepthMarkDSS, 1, DirLightVS, DirLightPS)
    ////////////////////////
    /*        CSM         */
    ////////////////////////
    pass P1
    {
        SetRasterizerState(cascadeRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Mesh_GS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
    }
    pass P2
    {
        SetRasterizerState(cascadeRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Model_GS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
    }
    pass P3
    {
        SetRasterizerState(cascadeRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Animation_GS()));
        SetGeometryShader(CompileShader(gs_5_0, GS_PreRender()));
        SetPixelShader(NULL);
    }
}
//2
technique11 T2_PointLight
{
    ///////////////////////
    /*    Point Light    */
    ///////////////////////
    pass P0
    {
    //    SetRasterizerState(WireFrameRS);
        SetRasterizerState(NoDepthClipFrontRS);
        SetBlendState(AdditiveAlphaBlend, float4(0, 0, 0, 0), 0xFF);
        SetDepthStencilState(NoDepthWGreateEqualDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, PointLightVS()));
        SetHullShader(CompileShader(hs_5_0, PointLightHS()));
        SetDomainShader(CompileShader(ds_5_0, PointLightDS()));
        SetPixelShader(CompileShader(ps_5_0, PointLightPS()));
    }
   
    ////////////////////////
    /* Point Light Shadow */
    ////////////////////////
    pass P1
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Mesh_GS()));
        SetGeometryShader(CompileShader(gs_5_0, PointShadowGenGS()));
        SetPixelShader(NULL);
    }
    pass P2
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Model_GS()));
        SetGeometryShader(CompileShader(gs_5_0, PointShadowGenGS()));
        SetPixelShader(NULL);
    }
    pass P3
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Animation_GS()));
        SetGeometryShader(CompileShader(gs_5_0, PointShadowGenGS()));
        SetPixelShader(NULL);
    }
    
    /////////////////////
    /*    WireFrame    */    
    pass P4_Wire
    {
        SetRasterizerState(WireFrameRS);
        SetVertexShader(CompileShader(vs_5_0, PointLightVS()));
        SetHullShader(CompileShader(hs_5_0, PointLightHS()));
        SetDomainShader(CompileShader(ds_5_0, PointLightDS()));
        SetPixelShader(CompileShader(ps_5_0, PointLightPS()));
    }
}
//3
technique11 T3_SpotLight
{
    ///////////////////////
    /*    Spot Light    */
    ///////////////////////
    pass P0
    {
        //SetRasterizerState(WireFrameRS);
        SetRasterizerState(NoDepthClipFrontRS);
        SetBlendState(AdditiveAlphaBlend, float4(0, 0, 0, 0), 0xFF);
        SetDepthStencilState(NoDepthWGreateEqualDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, SpotLightVS()));
        SetHullShader(CompileShader(hs_5_0, SpotLightHS()));
        SetDomainShader(CompileShader(ds_5_0, SpotLightDS()));
        SetPixelShader(CompileShader(ps_5_0, SpotLightPS()));
    }
    ////////////////////////
    /* Spot Light Shadow */
    ////////////////////////
    pass P1
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Mesh_Spot()));
        SetPixelShader(NULL);
    }
    pass P2
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Model_Spot()));
        SetPixelShader(NULL);
    }
    pass P3
    {
        SetRasterizerState(shadowRS);
        SetDepthStencilState(ShadowGenDSS, 0);
        SetVertexShader(CompileShader(vs_5_0, VS_Animation_Spot()));
        SetPixelShader(NULL);
    }
    /////////////////////
    /*    WireFrame    */    
    pass P4_Wire
    {
        SetRasterizerState(WireFrameRS);
        SetVertexShader(CompileShader(vs_5_0, SpotLightVS()));
        SetHullShader(CompileShader(hs_5_0, SpotLightHS()));
        SetDomainShader(CompileShader(ds_5_0, SpotLightDS()));
        SetPixelShader(CompileShader(ps_5_0, SpotLightPS()));
    }

}
technique11 T4//ech_CapsuleLight
{
    ///////////////////////
    /*    Capsule Light    */
    ///////////////////////
    pass P0
    {
        //SetRasterizerState(WireFrameRS);
        SetRasterizerState(NoDepthClipFrontRS);
        SetBlendState(AdditiveAlphaBlend, float4(0, 0, 0, 0), 0xFF);
        SetDepthStencilState(NoDepthWGreateEqualDSS, 1);
        SetVertexShader(CompileShader(vs_5_0, CapsuleLightVS()));
        SetHullShader(CompileShader(hs_5_0, CapsuleLightHS()));
        SetDomainShader(CompileShader(ds_5_0, CapsuleLightDS()));
        SetPixelShader(CompileShader(ps_5_0, CapsuleLightPS()));
    }
    /////////////////////
    /*    WireFrame    */    
    pass P1_Wire
    {
        SetRasterizerState(WireFrameRS);
        SetVertexShader(CompileShader(vs_5_0, CapsuleLightVS()));
        SetHullShader(CompileShader(hs_5_0, CapsuleLightHS()));
        SetDomainShader(CompileShader(ds_5_0, CapsuleLightDS()));
        SetPixelShader(CompileShader(ps_5_0, CapsuleLightPS()));
    }

}