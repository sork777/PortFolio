#include "Framework.h"
#include "Sky.h"
#include <climits>

Sky::Sky()
	:cubeSRV(NULL)
{
	Initialize();
}

Sky::Sky(wstring cubeMapFile)
{
	Initialize();	

	wstring temp = L"../../_Textures/" + cubeMapFile;
	Check(D3DX11CreateShaderResourceViewFromFile
	(
		D3D::GetDevice(),temp.c_str(),NULL,NULL,&cubeSRV,NULL
	));

	sCubeSRV = shader->AsSRV("SkyCubeMap");
}


Sky::~Sky()
{
	SafeDelete(sphere);
	SafeDelete(buffer);
	SafeRelease(sBuffer);
	SafeRelease(cubeSRV);
	SafeRelease(sCubeSRV);
	
}

void Sky::Initialize()
{
	shader = SETSHADER(L"022_Sky.fx");
	//perframe = new PerFrame(shader);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Sky");

	sphere = new MeshRender(shader, new MeshSphere(0.5f));
	sphere->AddInstance();
}

void Sky::Update()
{
	//ImGui::ColorEdit3("Sky Center", (float*)&desc.Center);
	//ImGui::ColorEdit3("Sky Apex", (float*)&desc.Apex);
	//ImGui::InputFloat("Sky Height", &desc.Height, 0.1f);
	//ImGui::Separator();


	Vector3 pos;
	
	if(false == Context::Get()->HasSubCam())
		Context::Get()->GetCamera()->Position(&pos);
	else
		Context::Get()->GetSubCamera()->Position(&pos);

	//perframe->Update();
	sphere->GetTransform(0)->Position(pos);
	sphere->UpdateTransforms();

	
}

void Sky::Render()
{
//	perframe->Render();
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Vector3 glDir = Context::Get()->LightDirection();

	shader->AsVector("glDir")->SetFloatVector(glDir);
	sCubeSRV->SetResource(cubeSRV);
	sphere->Pass(2);
	sphere->Render();
}
