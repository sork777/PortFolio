#include "stdafx.h"
#include "ColliderTestDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"


void ColliderTestDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);
	shader =SETSHADER(L"027_Animation.fx");

	Mesh();
	sky = new Sky(L"Environment/GrassCube1024.dds");
	brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");

	float width = D3D::Width();
	float height = D3D::Height();

	shader->AsSRV("SkyCubeMap")->SetResource(sky->CubeSRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());

	testCol = new OBBColliderTest();
	testCol->SetDebugModeOn();
	for (int i = 0; i < 2048; i++)
	{
		Vector3 randpos = Math::RandomVec3(-100.0f, 100.f);
		randpos.y = Math::Clamp(randpos.y, 1.5f, 100.0f);
		//Transform* trans = new Transform()
		testCol->AddInstance();
		testCol->GetTransform(i)->Position(randpos);
		testCol->GetTransform(i)->Scale(0.5f, 0.5f, 0.5f);
	}
	testCol->GetTransform(0)->Scale(5,5,5);

	perspective = new Perspective(D3D::Width(), D3D::Height(), 1.0f, 1000.0f, Math::PI * 0.25f);
	testCol->SetFrustum(new Frustum(NULL, perspective));
}

void ColliderTestDemo::Destroy()
{
}

void ColliderTestDemo::Update()
{
	


	sky->Update();

	testCol->Update();
	cube->Update();
	grid->Update();
	
	ImGuiTreeNodeFlags collapsFlag = ImGuiTreeNodeFlags_CollapsingHeader;
	if (ImGui::CollapsingHeader("Floor", collapsFlag))
		floor->Property();
	if (ImGui::CollapsingHeader("Stone", collapsFlag))
		stone->Property();

	static bool bRay = false;
	ImGui::Checkbox("Collision<->Ray", &bRay);

	if(bRay)
	{
		Matrix world;
		D3DXMatrixIdentity(&world);
		Vector3 start, direction;
		Matrix V = Context::Get()->View();
		Matrix P = Context::Get()->Projection();

		Vector3 mouse = Mouse::Get()->GetPosition();

		Vector3 n, f;//근면 원면

		//근면
		mouse.z = 0.0f;
		Context::Get()->GetViewport()->Unproject(&n, mouse, world, V, P);

		//원면
		mouse.z = 1.0f;
		Context::Get()->GetViewport()->Unproject(&f, mouse, world, V, P);

		//n에서 f로 쏜 방향
		direction = f - n;
		start = n;

		testCol->RayIntersect(start, direction);
	}
	else
	{
		testCol->ComputeColliderTest(0, 0);
	}
	Vector3 pos,rot;
	testCol->GetTransform()->Position(&pos);
	testCol->GetTransform()->RotationDegree(&rot);
	
	ImGui::SliderFloat3("Pos_0", (float*)&pos, -100, 100);
	ImGui::SliderFloat3("Rot_0", (float*)&rot, -180, 180);
	pos.y = Math::Clamp(pos.y, 2.5f, 20.0f);
	testCol->GetTransform()->Position(pos);
	testCol->GetTransform()->RotationDegree(rot);

}

void ColliderTestDemo::PreRender()
{
	
}
void ColliderTestDemo::Render()
{
	SceneRender();
	testCol->Render();
}

void ColliderTestDemo::Mesh()
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

	Transform* transform;
	cube = new MeshRender(shader, new MeshCube());
	cube->SetMaterial(stone);
	cube->AddInstance();
	transform = cube->GetTransform(0);
	transform->Position(0, 12.5f, 0);
	transform->Scale(15.0f, 5.0f, 25.0);
	cube->UpdateTransforms();

	grid = new MeshRender(shader, new MeshGrid(10, 10));
	grid->SetMaterial(floor);
	grid->AddInstance();
	transform = grid->GetTransform(0);
	transform->Position(0, 0, 0);
	transform->Scale(20, 1, 20);
	grid->UpdateTransforms();

}

void ColliderTestDemo::SceneRender()
{
	sky->Render();
	
	cube->Tech(tech);
	cube->Pass(pass);
	cube->Render();

	grid->Tech(tech);
	grid->Pass(pass);
	grid->Render();
}