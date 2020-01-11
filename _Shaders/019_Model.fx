#include "000_Header.fx"
#include "000_Light.fx"
#include "000_Model.fx"

float4 PS(MeshOutput input) : SV_Target0
{
	float4 diffuse = DiffuseMap.Sample(LinearSampler, input.Uv);

	float3 normal = normalize(input.Normal);
	float NdotL = saturate(dot(normal, -GlobalLight.Direction));

	return (diffuse * NdotL);
}

RasterizerState RS
{
	Fillmode = Wireframe;
};


technique11 T0
{
		P_VP(P0, VS_Mesh, PS)
		P_VP(P1, VS_Model, PS)

		P_RS_VP(P2, RS, VS_Mesh, PS)
		P_RS_VP(P3, RS, VS_Model, PS)
}