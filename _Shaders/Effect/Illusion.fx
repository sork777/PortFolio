#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"
#define MAX_ILLUSION_COUNT 100

float4 ILL_Color;

MeshOutput VS_ILL_Animation(VertexModel input)
{
    MeshOutput output;

    matrix mat;
    input.Transform = World;
    SetAnimationWorld(World, input);
    
    output.oPosition = input.Position.xyz;
    output.Position = WorldPosition(input.Position);
    output.wPosition = output.Position.xyz;

    output.Position = ViewProjection(output.Position);

    output.wvpPosition = output.Position;

    output.Normal = WorldNormal(input.Normal);

    output.Tangent = WorldTangent(input.Tangent);

    output.Uv = input.Uv;
        
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.ID = input.InstID;
    output.Clip = 0;

    return output;
}

float4 PS(MeshOutput input) : SV_Target0
{
    float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);
    Material.Diffuse *= diffuse;
    
    NormalMapping(input.Uv, input.Normal, input.Tangent);
    float NdotL = dot(normalize(input.Normal), -GlobalLight.Direction);
    //Material.Diffuse *= NdotL;
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    return float4(MaterialToColor(result), 1) * ILL_Color;
    //return float4(diffuse * NdotL, 1);
}

technique11 T0
{
    //P_VP(P0, VS_Mesh, PS)
	P_RS_BS_VP(P0, NoneRS, IllusionBlend, VS_ILL_Animation, PS)
}