#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

float4 MirrorClipPlane;
Texture2D MirrorMap;
Texture2D ReflectionMap;
matrix R;

cbuffer CB_MirrorBuffer
{
    matrix ReflectionMatrix;
    float4 RefractionColor;

    float MirrorShininess;
};

MeshOutput VS_Mesh_Reflect(VertexMesh input)
{
    MeshOutput output; // = VS_Mesh(input);
    output.oPosition = input.Position.xyz;
    
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;
    output.Clip = dot(WorldPosition(input.Position), MirrorClipPlane);
    output.Position = mul(output.Position,R);
  

    output.Position = ViewProjection(output.Position);
    output.wvpPosition = output.Position;

    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Uv = input.Uv;

    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    //output.Clip = 0;
    output.Position.y += 0.1f;
    output.ID = 0;
    return output;
}

MeshOutput VS_Model_Reflect(VertexModel input)
{
    MeshOutput output; // = VS_Model(input);
    SetModelWorld(World, input);

    output.oPosition = input.Position.xyz;
    
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = mul(output.Position, R);

    output.Position = ViewProjection(output.Position);
    output.wvpPosition = output.Position;
    
    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Uv = input.Uv;
    
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.ID = input.InstID;
    output.Clip = dot(WorldPosition(input.Position), MirrorClipPlane);
    output.Position.y += 0.1f;
    return output;
}

MeshOutput VS_Animation_Reflect(VertexModel input)
{
    //MeshOutput output = VS_Animation(input);
    MeshOutput output;

    SetAnimationWorld(World, input);

    output.oPosition = input.Position.xyz;
    
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = mul(output.Position, R);

    output.Position = ViewProjection(output.Position);
    output.wvpPosition = output.Position;
    
    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Uv = input.Uv;
    
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.ID = input.InstID;
    output.Clip = dot(WorldPosition(input.Position), MirrorClipPlane);
    output.Position.y += 0.1f;
    return output;
}
float4 PS(MeshOutput input) : SV_Target0
{
    return VS_Shadow(input, VS_AllLight(input));
}
////////////////////////////////////////////////////////////////////////////////

struct VertexOutput
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float4 ReflectionPosition : Position2;

    float2 Uv : Uv0;
};

VertexOutput VS_Mirror(VertexTexture input)
{
    VertexOutput output;
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = ViewProjection(output.Position);

    matrix reflection = mul(World, mul(ReflectionMatrix, Projection));
    output.ReflectionPosition = mul(input.Position, reflection);

    output.Uv = input.Uv;
    
    return output;
}


float4 PS_Mirror(MeshOutput input) : SV_Target0
{
    float3 viewDirection = normalize(ViewPosition() - input.wPosition);
    float3 color = MirrorMap.Sample(LinearSampler, input.Uv);;
    //float3 reflection = ReflectionMap.Sample(LinearSampler, input.Uv);;
    //color = lerp(color, reflection, 0.2f);
    float3 light = -GlobalLight.Direction;
    //light.x *= -1.0f;
    
    float3 NdotL = reflect(light, input.Normal);

    //color = DiffuseMap.Sample(LinearSampler, input.Uv);
    return float4(color, 0.20f);

}


RasterizerState RS
{
    CullMode = Front;
};

RasterizerState RS_Clock
{
    FrontCounterClockWise = true;
};

technique11 T0
{
    /* Depth */
    P_RS_VP(P0, RS, VS_Depth_Mesh, PS_Depth)
    P_RS_VP(P1, RS, VS_Depth_Model, PS_Depth)
    P_RS_VP(P2, RS, VS_Depth_Animation, PS_Depth)
   
    /* Reflect Render */
    P_RS_DSS_Ref_VP(P3, RS_Clock,reflectDSS, 1, VS_Mesh_Reflect, PS)
    P_RS_DSS_Ref_VP(P4, RS_Clock,reflectDSS, 1, VS_Model_Reflect, PS)
    P_RS_DSS_Ref_VP(P5, RS_Clock,reflectDSS, 1, VS_Animation_Reflect, PS)
    /* Render */
    P_VP(P6, VS_Mesh, PS)
    P_VP(P7, VS_Model, PS)
    P_VP(P8, VS_Animation, PS)
    
    P_DSS_Ref_BS_VP(P9, mirrorDSS, 1, NoRenderTargetBlend, VS_Mesh, PS_Mirror)
    P_BS_VP(P10, TransparentBlend, VS_Mesh, PS_Mirror)
}