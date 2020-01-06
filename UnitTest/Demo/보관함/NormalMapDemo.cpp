#include "stdafx.h"
#include "NormalMapDemo.h"

void NormalMapDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"029_NormalMap.fx");

	PLIcon = new Texture(L"Icon/light.png");
	SLIcon = new Texture(L"Icon/spot.png");
	CLIcon = new Texture(L"Icon/capsule.png");
	//CreateMesh
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->NormalMap("Floor_Normal.png");
		floor->SpecularMap("Floor_Specular.png");
		floor->Diffuse(1, 1, 1, 1);

		floor->Specular(1, 1, 1, 20);


		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");
		stone->NormalMap("Stones_Normal.png");
		stone->SpecularMap("Stones_Specular.png");
		stone->Diffuse(1, 1, 1, 1);


		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");
		brick->NormalMap("Bricks_Normal.png");
		brick->SpecularMap("Bricks_Specular.png");
		brick->Diffuse(1, 1, 1, 1);
		brick->Specular(1, 0.3f, 0.3f, 20);

		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
		wall->NormalMap("Wall_Normal.png");
		wall->SpecularMap("Wall_Specular.png");

		wall->Diffuse(1, 1, 1, 1);
		wall->Specular(1, 1, 1, 20);

		cube = new MeshCube(shader);
		cube->GetTransform()->Position(0, 2.5f, 0);
		cube->GetTransform()->Scale(15.0f, 5.0f, 25.0);

		grid = new MeshGrid(shader, 3, 3);
		grid->GetTransform()->Position(0, 0, 0);
		grid->GetTransform()->Scale(20, 1, 20);


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



void NormalMapDemo::Update()
{
	ImGui::SliderFloat3("LightDir", (float*)&Context::Get()->LightDirection(), -1, 1);

	static UINT selected = 0;
	ImGui::InputInt("Selected", (int*)&selected);
	selected %= 3;
	shader->AsScalar("Selected")->SetInt(selected);

	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Update();
		cylinder[i]->Update();
	}

	cube->Update();
	grid->Update();
	modelRender->Update();	

	/* 프로젝션용 */
	Vector3 position;
	Vector3 project;

	Matrix world;
	D3DXMatrixIdentity(&world);
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

	/* 라이트별 선택 번호 */
	static int selectedPL = -1;
	static int selectedSL = -1;
	static int selectedCL = -1;
	/* 라이트의 갯수*/
	int count = Context::Get()->PointLightsCount();
	ImGui::InputInt("Selected Point Light", (int*)&selectedPL);
	/* 있으면 갯수 아니면 -1*/
	if (count > 0)
		selectedPL %= count;
	else
		selectedPL = -1;

	/* 마우스 좌표 */
	Vector3 mouse;
	if(Mouse::Get()->Down(0)) mouse =Mouse::Get()->GetPosition();
	/* 픽킹 체크용 클릭 하면 false로 */
	bool bClickCheck = !Mouse::Get()->Down(0);
	/* 아이콘 출력 */
	for (int i = 0; i < count; i++)
	{
		position = Context::Get()->GetPointLight(i).Position;
		Context::Get()->GetViewport()->Project(&project, position, world, V, P);
		Gui::Get()->RenderGUITexture(Vector2(project.x,project.y),Vector2(60,60), Context::Get()->GetPointLight(i).Diffuse, PLIcon);
		if (bClickCheck == false)
		{
			bClickCheck  = mouse.x <= project.x +  30.0f;
			bClickCheck &= mouse.y <= project.y + 30.0f;
			bClickCheck &= mouse.x >= project.x - 30.0f;
			bClickCheck &= mouse.y >= project.y - 30.0f;
			if (bClickCheck) selectedPL = i;
		}
	}

	ImGui::InputInt("Selected Spot Light", (int*)&selectedSL);
	count = Context::Get()->SpotLightsCount();
	if (count > 0)
		selectedSL %= count;
	else
		selectedSL = -1;

	/* 라이트 마다 초기화 */
	bClickCheck = !Mouse::Get()->Down(0);
	for (int i = 0; i < count; i++)
	{
		position = Context::Get()->GetSpotLight(i).Position;
		Context::Get()->GetViewport()->Project(&project, position, world, V, P);
		Gui::Get()->RenderGUITexture(Vector2(project.x, project.y), Vector2(50, 50), Context::Get()->GetSpotLight(i).Diffuse, SLIcon);

		if (bClickCheck == false)
		{
			bClickCheck = mouse.x <= project.x + 25.0f;
			bClickCheck &= mouse.y <= project.y + 25.0f;
			bClickCheck &= mouse.x >= project.x - 25.0f;
			bClickCheck &= mouse.y >= project.y - 25.0f;
			if (bClickCheck) selectedSL = i;
		}
	}

	ImGui::InputInt("Selected Capsule Light", (int*)&selectedCL);
	count = Context::Get()->CapsuleLightsCount();
	if (count > 0)
		selectedCL %= count;
	else
		selectedCL = -1;

	bClickCheck = !Mouse::Get()->Down(0);
	for (int i = 0; i < count; i++)
	{
		position = Context::Get()->GetCapsuleLight(i).Position + Context::Get()->GetCapsuleLight(i).Direction*Context::Get()->GetCapsuleLight(i).Length*0.5f;
		Context::Get()->GetViewport()->Project(&project, position, world, V, P);
		Gui::Get()->RenderGUITexture(Vector2(project.x, project.y), Vector2(100, 100), Context::Get()->GetCapsuleLight(i).Diffuse, CLIcon);
		
		if (bClickCheck == false)
		{
			bClickCheck  = mouse.x <= project.x + 50.0f;
			bClickCheck &= mouse.y <= project.y + 50.0f;
			bClickCheck &= mouse.x >= project.x - 50.0f;
			bClickCheck &= mouse.y >= project.y - 50.0f;
			if (bClickCheck) selectedCL = i;
		}
	}

	/* 라이트 생성 */
	if (ImGui::Button("PL create"))
	{
		PointLight light;
		light = {
			Color(0.0f,0.2f, 0.2f, 1.0f),
			Color(0.0f,0.0f, 1.0f, 1.0f),
			Color(0.0f,0.0f, 0.7f, 1.0f),
			Vector3(-25,5,0),25.0f,0.25f
		};
		Context::Get()->AddPointLight(light);
		/* 선택 번호 조정 */
		selectedPL = Context::Get()->PointLightsCount() - 1;
	}
	if (ImGui::Button("SL create"))
	{
		SpotLight light;
		light = {
			Color(0.0f,0.0f, 0.0f, 1.0f),
			Color(0.0f,1.0f, 0.0f, 1.0f),
			Color(0.0f,0.7f, 0.0f, 1.0f),
			Vector3(30,50,0),50.0f,
			Vector3(0,-1,0),30.0f,0.5f
		};
		Context::Get()->AddSpotLight(light);
		selectedSL = Context::Get()->SpotLightsCount() - 1;

	}
	if (ImGui::Button("CL create"))
	{
		CapsuleLight light;
		light = {
			Color(1.0f,1.0f, 1.0f, 1.0f),
			Color(1.0f,0.0f, 0.0f, 1.0f),
			Color(0.8f,0.0f, 0.0f, 1.0f),
			Vector3(-25.0f,5,-20),10.0f,
			Vector3(1,0,0),50.0f,0.1f
		};
		Context::Get()->AddCapsuleLight(light);
		selectedCL = Context::Get()->CapsuleLightsCount() - 1;
	}

	

	/* 라이트 속성 조정 함수 */
	PointLightController(selectedPL);
	SpotLightController(selectedSL);
	CapsuleLightController(selectedCL);

	/* 영역 표기 */
	PointLightArea(selectedPL);
	SpotLightArea(selectedSL);
	CapsuleLightArea(selectedCL);
}

void NormalMapDemo::Render()
{
	wall->Render();
	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Pass(0);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < 10; i++)
	{
		cylinder[i]->Pass(0);
		cylinder[i]->Render();
	}
	stone->Render();
	cube->Pass(0);
	cube->Render();
	floor->Render();
	grid->Pass(0);
	grid->Render();

	modelRender->Pass(1);
	modelRender->Render();
}

void NormalMapDemo::PointLightArea(int index)
{
	if (index < 0)return;

	UINT stackCount = 60;
	float thetaStep = 2.0f * Math::PI / stackCount;
	Vector3 pos = Context::Get()->GetPointLight(index).Position;
	float radius = Context::Get()->GetPointLight(index).Range;

	vector<Vector3> v, v2, v3;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(radius * cosf(theta)),
			0,
			(radius * sinf(theta))
		);
		Vector3 p2 = Vector3
		(
			(radius * cosf(theta)),
			(radius * sinf(theta)),
			0
		);
		Vector3 p3 = Vector3
		(
			0,
			(radius * cosf(theta)),
			(radius * sinf(theta))
		);
		p += pos;
		p2 += pos;
		p3 += pos;
		v.emplace_back(p);
		v2.emplace_back(p2);
		v3.emplace_back(p3);
	}
	for (UINT i = 0; i < stackCount; i++)
	{
		DebugLine::Get()->RenderLine(v[i], v[i + 1]);
		DebugLine::Get()->RenderLine(v2[i], v2[i + 1]);
		DebugLine::Get()->RenderLine(v3[i], v3[i + 1]);
	}
}

void NormalMapDemo::SpotLightArea(int index)
{
	if (index < 0)return;

	UINT stackCount = 60;
	float thetaStep = 2.0f * Math::PI / stackCount;
	Vector3 pos = Context::Get()->GetSpotLight(index).Position;
	float dist = Context::Get()->GetSpotLight(index).Range;
	float angle = Context::Get()->GetSpotLight(index).Angle;
	float radius = dist * tan(Math::ToRadian(angle));
	Vector3 dir = Context::Get()->GetSpotLight(index).Direction;
	
	Vector3 axis;
	D3DXVec3Cross(&axis, &Vector3(0, 0, 1), &dir);
	float radian = D3DXVec3Dot(&Vector3(0, 0, 1), &dir)-Math::PI*0.5f;

	Matrix R;
	D3DXMatrixRotationAxis(&R, &axis, radian);

	vector<Vector3> v;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(radius * cosf(theta)),
			(radius * sinf(theta)),
			0
		);
		
		D3DXVec3TransformCoord(&p, &p, &R);
		p += pos;
		p += dir*dist;
		v.emplace_back(p);
	}
	
	DebugLine::Get()->RenderLine(pos, v[stackCount / 4]);
	DebugLine::Get()->RenderLine(pos, v[stackCount/2]);
	DebugLine::Get()->RenderLine(pos, v[stackCount * 3 / 4]);
	DebugLine::Get()->RenderLine(pos, v[stackCount]);


	for (UINT i = 0; i < stackCount; i++)
	{
		DebugLine::Get()->RenderLine(v[i], v[i + 1]);
	}
}

void NormalMapDemo::CapsuleLightArea(int index)
{
	if (index < 0)return;

	UINT stackCount = 60;
	float thetaStep = 2.0f * Math::PI / stackCount;
	Vector3 start = Context::Get()->GetCapsuleLight(index).Position;
	Vector3 dir = Context::Get()->GetCapsuleLight(index).Direction;
	Vector3 end = dir *Context::Get()->GetCapsuleLight(index).Length;
	float radius = Context::Get()->GetCapsuleLight(index).Range;

	Vector3 axis;
	D3DXVec3Cross(&axis, &Vector3(0, 0, 1), &dir);
	float radian = D3DXVec3Dot(&Vector3(0, 0, 1), &dir) -Math::PI*0.5f;

	Matrix R;
	D3DXMatrixRotationAxis(&R, &axis, radian);

	vector<Vector3> v, v2, v3;
	vector<Vector3> e, e2, e3;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(radius * cosf(theta)),
			0,
			(radius * sinf(theta))
		);
		Vector3 p2 = Vector3
		(
			0,
			(radius * cosf(theta)),
			(radius * sinf(theta))
		);
		Vector3 p3 = Vector3
		(
			(radius * cosf(theta)),
			(radius * sinf(theta)),
			0
		);
		D3DXVec3TransformCoord(&p, &p, &R);
		D3DXVec3TransformCoord(&p2, &p2, &R);
		D3DXVec3TransformCoord(&p3, &p3, &R);
		p += start;
		p2 += start;
		p3 += start;
		v.emplace_back(p);
		v2.emplace_back(p2);
		v3.emplace_back(p3);

		p += end;
		p2 += end;
		p3 += end;
		e.emplace_back(p);
		e2.emplace_back(p2);
		e3.emplace_back(p3);
	}
	for (UINT i = 0; i < stackCount; i++)
	{
		if (i <= stackCount / 2)
		{
			DebugLine::Get()->RenderLine(v[i], v[i + 1]);
			DebugLine::Get()->RenderLine(v2[i], v2[i + 1]);
		}
		DebugLine::Get()->RenderLine(v3[i], v3[i + 1]);
	}
	DebugLine::Get()->RenderLine(v3[stackCount / 4], e3[stackCount / 4]);
	DebugLine::Get()->RenderLine(v3[stackCount / 2], e3[stackCount / 2]);
	DebugLine::Get()->RenderLine(v3[stackCount*3 / 4], e3[stackCount*3 / 4]);
	DebugLine::Get()->RenderLine(v3[stackCount], e3[stackCount]);

	for (UINT i = 0; i < stackCount; i++)
	{
		if (i >= stackCount / 2)
		{
			DebugLine::Get()->RenderLine(e[i], e[i + 1]);
			DebugLine::Get()->RenderLine(e2[i], e2[i + 1]);
		}
		DebugLine::Get()->RenderLine(e3[i], e3[i + 1]);
	}

}

void NormalMapDemo::PointLightController(int index)
{
	if (index < 0)return;

	if (ImGui::CollapsingHeader("PointLight Info"))
	{
		ImGui::ColorEdit3("PointLight Specular", (float*)&Context::Get()->GetPointLight(index).Specular);
		ImGui::ColorEdit3("PointLight Diffuse", (float*)&Context::Get()->GetPointLight(index).Diffuse);
		ImGui::SliderFloat3("PointLight Position", (float*)&Context::Get()->GetPointLight(index).Position, -100.0f, 100.0f);
		ImGui::SliderFloat("PointLight Intensity", &Context::Get()->GetPointLight(index).Intensity, 0.01f, 1.0f);
		ImGui::SliderFloat("PointLight Range", &Context::Get()->GetPointLight(index).Range, 1.0f, 200.0f);
	}
}

void NormalMapDemo::SpotLightController(int index)
{
	if (index < 0)return;

	if (ImGui::CollapsingHeader("SpotLight Info"))
	{
		ImGui::ColorEdit3("SpotLight Specular", (float*)&Context::Get()->GetSpotLight(index).Specular);
		ImGui::ColorEdit3("SpotLight Diffuse", (float*)&Context::Get()->GetSpotLight(index).Diffuse);
		ImGui::SliderFloat3("SpotLight Position", (float*)&Context::Get()->GetSpotLight(index).Position, -100.0f, 100.0f);
		ImGui::SliderFloat3("SpotLight Direction", (float*)&Context::Get()->GetSpotLight(index).Direction, -1.0f, 1.0f);
		ImGui::SliderFloat("SpotLight Intensity", &Context::Get()->GetSpotLight(index).Intensity, 0.01f, 1.0f);
		ImGui::SliderFloat("SpotLight Range", &Context::Get()->GetSpotLight(index).Range, 1.0f, 200.0f);
		ImGui::SliderFloat("SpotLight Angle", &Context::Get()->GetSpotLight(index).Angle, 0.1f, 50.0f);
	}
}

void NormalMapDemo::CapsuleLightController(int index)
{
	if (index < 0)return;

	if (ImGui::CollapsingHeader("CapsuleLight Info"))
	{
		ImGui::ColorEdit3("CapsuleLight Specular", (float*)&Context::Get()->GetCapsuleLight(index).Specular);
		ImGui::ColorEdit3("CapsuleLight Diffuse", (float*)&Context::Get()->GetCapsuleLight(index).Diffuse);
		ImGui::SliderFloat3("CapsuleLight Position", (float*)&Context::Get()->GetCapsuleLight(index).Position, -100.0f, 100.0f);
		ImGui::SliderFloat3("CapsuleLight Direction", (float*)&Context::Get()->GetCapsuleLight(index).Direction, -1.0f, 1.0f);
		ImGui::SliderFloat("CapsuleLight Intensity", &Context::Get()->GetCapsuleLight(index).Intensity, 0.01f, 1.0f);
		ImGui::SliderFloat("CapsuleLight Range", &Context::Get()->GetCapsuleLight(index).Range, 1.0f, 50.0f);
		ImGui::SliderFloat("CapsuleLight Angle", &Context::Get()->GetCapsuleLight(index).Length, 1.0f, 250.0f);
	}
}
