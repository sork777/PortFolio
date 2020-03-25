#include "stdafx.h"
#include "CSMPrac.h"


void CSMPrac::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"HW04_CSM.fx");
	shadow = new CSM(shader);

	float width = D3D::Width();
	float height = D3D::Height();


	{
		depth2D = new Render2D();
		depth2D->GetTransform()->Position(100, height - 100, 0);
		depth2D->GetTransform()->Scale(200, 200, 1);
	}


	//CreateMesh
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->NormalMap("Floor_Normal.png");
		floor->SpecularMap("Floor_Specular.png");


		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");
		stone->NormalMap("Stones_Normal.png");
		stone->SpecularMap("Stones_Specular.png");


		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");
		brick->NormalMap("Bricks_Normal.png");
		brick->SpecularMap("Bricks_Specular.png");

		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
		wall->NormalMap("Wall_Normal.png");
		wall->SpecularMap("Wall_Specular.png");


		cube = new MeshCube(shader);
		cube->GetTransform()->Position(0, 2.5f, 0);
		cube->GetTransform()->Scale(15.0f, 5.0f, 25.0);

		grid = new MeshGrid(shader, 9, 9);
		grid->GetTransform()->Position(0, 0, 0);
		grid->GetTransform()->Scale(30, 1, 30);


		for (UINT i = 0; i < 5; i++)
		{
			cylinder[i * 2] = new MeshCylinder(shader, 0.5f, 3.0f,false, 20, 20);
			cylinder[i * 2]->GetTransform()->Position(-30, 6.0f, -15.0f + (float)i * 15.0f);
			cylinder[i * 2]->GetTransform()->Scale(5, 5, 5);

			cylinder[i * 2 + 1] = new MeshCylinder(shader, 0.5f, 3.0f, false, 20, 20);
			cylinder[i * 2 + 1]->GetTransform()->Position(30, 6.0f, -15.0f + (float)i * 15.0f);
			cylinder[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);


			sphere[i * 2] = new MeshSphere(shader, 0.5f, false, 20, 20);
			sphere[i * 2]->GetTransform()->Position(-30.0f, 15.5f, -15.0f + i * 15.0f);
			sphere[i * 2]->GetTransform()->Scale(5, 5, 5);

			sphere[i * 2 + 1] = new MeshSphere(shader, 0.5f, false, 20, 20);
			sphere[i * 2 + 1]->GetTransform()->Position(30.0f, 15.5f, -15.0f + i * 15.0f);
			sphere[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);
		}
	}

	//Load Model
	{
		model = new Model();
		model->ReadMaterial(L"Eclipse/Eclipse");
		model->ReadMesh(L"Eclipse/Eclipse");

		modelRender = new ModelRender(shader, model);
		modelRender->AddTransform();
		modelRender->GetTransform(0)->Position(0,4.25f,0);
		modelRender->GetTransform(0)->Scale(0.3f, 0.3f, 0.3f);
		modelRender->UpdateTransform();
	}

}



void CSMPrac::Update()
{
	Vector3 pos;
	Context::Get()->GetCamera()->Position(&pos);

	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Update();
		cylinder[i]->Update();
	}

	cube->Update();
	grid->Update();
	modelRender->Update();	
}

void CSMPrac::Render()
{
	RenderObj();	
}

void CSMPrac::PreRender()
{
	meshPass = 0;
	modelPass = 1;
	animPass = 2;

	shadow->Set();
	wall->Render();
	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < 10; i++)
	{
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}

	stone->Render();
	cube->Pass(meshPass);
	cube->Render();
	floor->Render();
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Pass(modelPass);
	modelRender->Render();
	
}

void CSMPrac::PostRender()
{
	//depth2D->SRV(shadow->SRV());
	//depth2D->Pass(6);
	//depth2D->Render();
	
}

void CSMPrac::RenderObj()
{
	meshPass = 3;
	modelPass = 4;
	animPass = 5;

	wall->Render();
	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < 10; i++)
	{
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}

	stone->Render();
	cube->Pass(meshPass);
	cube->Render();
	floor->Render();
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Pass(modelPass);
	modelRender->Render();
}
