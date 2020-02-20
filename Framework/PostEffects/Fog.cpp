#include "Framework.h"
#include "Fog.h"


Fog::Fog()
	:Render2D(L"Fog.fx")
{
	fogBuffer = new ConstantBuffer(&fogDesc, sizeof(FogDesc));
	sFogBuffer = shader->AsConstantBuffer("CB_Fog");

	fogDesc.FogColor	= Color(1, 1, 1, 1);
	fogDesc.FogDistance = Vector2(10.0f, 10.0f);
	fogDesc.FogDensity	= 0.5f;
	fogDesc.FogType		= 2;
}


Fog::~Fog()
{
	SafeDelete(fogBuffer);
}

void Fog::Update()
{
	Super::Update();
	Vector3 camPos;
	Context::Get()->GetCamera()->Position(&camPos);

	Matrix proj = Context::Get()->Projection();
	perspertive.x = 1.0f / proj.m[0][0];
	perspertive.y = 1.0f / proj.m[1][1];
	perspertive.z = proj.m[3][2];
	perspertive.w = -proj.m[2][2];

	shader->AsVector("PerspectiveValues")->SetFloatVector(perspertive);
}

void Fog::Render()
{
	fogBuffer->Apply();
	sFogBuffer->SetConstantBuffer(fogBuffer->Buffer());
	if(depthsrv !=NULL)
		shader->AsSRV("DepthMap")->SetResource(depthsrv);
	Super::Render();
}

void Fog::Property()
{
	ImGui::PushID(this);
	{
		ImGui::Text("Fog");
		ImGui::ColorEdit4("FogColor", (float*)&fogDesc.FogColor);
		ImGui::SliderFloat2("FogDistance", (float*)&fogDesc.FogDistance,1.0f,200.0f);
		ImGui::SliderFloat("FogDensity", &fogDesc.FogDensity,0.1f,5.0f);
		ImGui::SliderInt("FogType", (int*)&fogDesc.FogType,0,3);
	}
	ImGui::PopID();
}

void Fog::SetDepthSrv(ID3D11ShaderResourceView* depth)
{
	depthsrv = depth;
}
