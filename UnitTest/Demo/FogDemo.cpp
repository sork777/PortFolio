#include "stdafx.h"
#include "FogDemo.h"
#include "Viewer/Freedom.h"
#include "PostEffects/Fog.h"
#include "Environment/Sky.h"


void FogDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);
	shader = new Shader(L"027_Animation.fx");

	Mesh();
	sky = new Sky(L"Environment/GrassCube1024.dds");
	brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");

	float width = D3D::Width();
	float height = D3D::Height();

	fog = new Fog();

	fog->GetTransform()->Position(width*0.5f, height*0.5f, 0);
	fog->GetTransform()->Scale(width, height, 1);

	render2D = new Render2D();
	render2D->GetTransform()->Position(width*0.25f, height*0.5f, 0);
	render2D->GetTransform()->Scale(width*0.5f, height, 1);
	
	finalRender2D = new Render2D();
	finalRender2D->GetTransform()->Position(width*0.75f, height*0.5f, 0);
	finalRender2D->GetTransform()->Scale(width*0.5f, height, 1);

	for (UINT i = 0; i < 2; i++)
		target[i] = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil((UINT)width, (UINT)height);

	viewport = new Viewport((UINT)width, (UINT)height);

	shader->AsSRV("SkyCubeMap")->SetResource(sky->CubeSRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());
}

void FogDemo::Destroy()
{
}

void FogDemo::Update()
{
	//Property();
	sky->Update();
	static bool bTess = false;
	static bool bWire = false;
	ImGui::Checkbox("TessTest", &bTess);
	ImGui::Checkbox("Wire", &bWire);
	tech = bWire ? 2 : 1;

	//displacement 매핑이 무지 안된다.
	// 뚜껑 열리는 문제가 있음...
	if (bTess)
	{
		pass = 1;
		cube->GetMesh()->Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		grid->GetMesh()->Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		sphere->GetMesh()->Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		cylinder->GetMesh()->Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		
		cylinder->GetMesh()->Tessellation(true);
		sphere->GetMesh()->Tessellation(true);
		grid->GetMesh()->Tessellation(true);
		cylinder->GetMesh()->Displacement(0.2f);
		grid->GetMesh()->Displacement(1.5f);
		cube->GetMesh()->Tessellation(true);
		cube->GetMesh()->Displacement(0.5f);
	}
	else
	{ 
		pass = 0;
		sphere->GetMesh()->Topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cylinder->GetMesh()->Topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cube->GetMesh()->Topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		grid->GetMesh()->Topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		cylinder->GetMesh()->Tessellation(false);
		sphere->GetMesh()->Tessellation(false);
		grid->GetMesh()->Tessellation(false);
		cube->GetMesh()->Tessellation(false);
	}

	sphere->Update();
	cylinder->Update();
	cube->Update();
	grid->Update();
	
	fog->Update();
	fog->Property();

	ImGuiTreeNodeFlags collapsFlag = ImGuiTreeNodeFlags_CollapsingHeader;
	if (ImGui::CollapsingHeader("Floor", collapsFlag))
		floor->Property();
	if (ImGui::CollapsingHeader("Stone", collapsFlag))
		stone->Property();
	if (ImGui::CollapsingHeader("Brick", collapsFlag))
		brick->Property();
	if (ImGui::CollapsingHeader("Wall", collapsFlag))
		wall->Property();
}

void FogDemo::PreRender()
{
	target[0]->Set(depthStencil->DSV());
	viewport->RSSetViewport();
	SceneRender();
	
	fog->SetDepthSrv(depthStencil->SRV());
	target[1]->Set(NULL);
	fog->SRV(target[0]->SRV());
	fog->Pass(0);
	fog->Render();
}
void FogDemo::Render()
{
	render2D->SRV(target[0]->SRV());
	render2D->Render();

	finalRender2D->SRV(target[1]->SRV());
	finalRender2D->Render();

}

void FogDemo::Mesh()
{
	floor = new Material(shader);
	floor->LoadDiffuseMap("Floor.png");
	floor->LoadNormalMap("Floor_Normal.png");
	floor->LoadSpecularMap("Floor_Specular.png");
	floor->LoadHeightMap("Floor_Displacement.png");

	stone = new Material(shader);
	stone->LoadDiffuseMap("Stones.png");
	stone->LoadNormalMap("Stones_Normal.png");
	stone->LoadSpecularMap("Stones_Specular.png");
	stone->LoadHeightMap("Stones_Displacement.png");

	brick = new Material(shader);
	brick->LoadDiffuseMap("Bricks.png");
	brick->LoadNormalMap("Bricks_Normal.png");
	brick->LoadSpecularMap("Bricks_Specular.png");
	brick->LoadHeightMap("Bricks_Displacement.png");

	wall = new Material(shader);
	wall->LoadDiffuseMap("Wall.png");
	wall->LoadNormalMap("Wall_Normal.png");
	wall->LoadSpecularMap("Wall_Specular.png");
	wall->LoadHeightMap("Wall_Displacement.png");
	//wall->LoadDiffuseMap("AluminiumInsulator_Albedo.png", "../../_Textures/MaterialPBR/");
	//wall->LoadNormalMap("AluminiumInsulator_Normal.png", "../../_Textures/MaterialPBR/");
	//wall->LoadSpecularMap("AluminiumInsulator_Roughness.png", "../../_Textures/MaterialPBR/");

	Transform* transform;
	cube = new MeshRender(shader, new MeshCube());
	cube->SetMaterial(stone);
	transform = cube->AddTransform();
	transform->Position(0, 12.5f, 0);
	transform->Scale(15.0f, 5.0f, 25.0);
	cube->UpdateTransforms();

	grid = new MeshRender(shader, new MeshGrid(10, 10));
	grid->SetMaterial(floor);
	transform = grid->AddTransform();
	transform->Position(0, 0, 0);
	transform->Scale(20, 1, 20);
	grid->UpdateTransforms();


	cylinder = new MeshRender(shader, new MeshCylinder(0.5f, 3.0f, 20, 20));
	cylinder->SetMaterial(brick);
	sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
	sphere->SetMaterial(stone);
	for (UINT i = 0; i < 5; i++)
	{
		transform = cylinder->AddTransform();
		transform->Position(-30, 6.0f, -30.0f + (float)i * 15.0f);
		transform->Scale(5, 5, 5);

		transform = cylinder->AddTransform();
		transform->Position(30, 6.0f, -30.0f + (float)i * 15.0f);
		transform->Scale(5, 5, 5);

		transform = sphere->AddTransform();
		transform->Position(-30, 15.5f, -30.0f + (float)i * 15.0f);
		transform->Scale(5, 5, 5);

		transform = sphere->AddTransform();
		transform->Position(30, 15.5f, -30.0f + (float)i * 15.0f);
		transform->Scale(5, 5, 5);
	}
	cylinder->UpdateTransforms();
	sphere->UpdateTransforms();
}

void FogDemo::SceneRender()
{
	sky->Render();
	
	sphere->Tech(tech);
	sphere->Pass(pass);
	sphere->Render();
	
	cylinder->Tech(tech);
	cylinder->Pass(pass);
	cylinder->Render();
	
	cube->Tech(tech);
	cube->Pass(pass);
	cube->Render();

	grid->Tech(tech);
	grid->Pass(pass);
	grid->Render();
}