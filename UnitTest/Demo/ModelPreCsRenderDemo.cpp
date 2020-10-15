#include "stdafx.h"
#include "ModelPreCsRenderDemo.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Terrain/TerrainLod.h"

#include "Editor/TerrainEditor.h"


ModelPreCsRenderDemo::ModelPreCsRenderDemo()
{
}


ModelPreCsRenderDemo::~ModelPreCsRenderDemo()
{
}

void ModelPreCsRenderDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);
	shader = SETSHADER(L"027_Animation.fx");
	//shader = SETSHADER(L"HW02_Deferred.fx");
	//Deffered Cascade shadow
	{
		//shadow = new CSM(shader);
		//gBuffer = new GBuffer(shader);
	}
	//Sky
	{
		skyShader = SETSHADER(L"045_Scattering.fx");
		sky = new Atmosphere(skyShader);

	}
	//Terrain
	{
		terrainShader = SETSHADER(L"047_TerrainLod.fx");

		TerrainLod::InitializeInfo info =
		{
			terrainShader,
			1.0f, 64, 40,
		};
		terrain = new TerrainLod(info);
		terrain->SetDebugMode(true);
		terrain->SetDefferedMode(true);

		terrainEditor = new TerrainEditor(terrain);
		terrainMat = new Material(terrainShader);
	}

	//ssao = new SSAO();
	//CreateBaseModels();
	CreateBaseActor();
	//PBR°ü·Ã
	cubeTex = new TextureCube((Vector3&)Vector3(0, 0, 0), 512, 512);
	Texture* brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	shader->AsSRV("SkyCubeMap")->SetResource(cubeTex->SRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());
	actorEditor = new ActorEditor();

}

void ModelPreCsRenderDemo::Ready()
{
}

void ModelPreCsRenderDemo::Destroy()
{
	SafeDelete(cubeTex);
	//SafeDelete(ssao);
	SafeDelete(terrainEditor);
	SafeDelete(terrain);
}

void ModelPreCsRenderDemo::Update()
{
	Context::Get()->ShowContextInfo();
	sky->Update();
	//gBuffer->Update();
	terrain->Update();
	for (Actor* actor : actors)
	{
		ImGui::PushID(actor);
		actor->Update();
		UINT curInstSize = actor->GetInstSize();

		string str = String::ToString(actor->GetName()) + "_ADD";
		if (ImGui::Button(str.c_str()))
		{
			Vector3 pos = Math::RandomVec3(-100, 100);
			pos.y = terrain->GetPickedHeight(pos);
			actor->AddInstanceData();
			actor->GetTransform(curInstSize)->Position(pos);
		}
		str = String::ToString(actor->GetName()) + "_Editor";
		if (ImGui::Button(str.c_str()) && false == bEditMode)
		{
			bEditMode = true;
			actorEditor->SetActor(actor);
		}

		UINT curInst = actorInstMap[actor];
		if (ImGui::SliderInt("CurInst", (int*)&curInst, 0, curInstSize - 1))
			actorInstMap[actor] = curInst;
		int curclip = actor->GetCurClip();
		int maxclip = actor->GetMaxClip() - 1;
		if (ImGui::SliderInt("ClipNum", &curclip, 0, maxclip))
			actor->SetMainClip(curclip);

		ImGui::PopID();
		ImGui::Separator();
	}

	if (true == bEditMode)
		actorEditor->Update();
	bEditMode = actorEditor->IsOpenedEditor();
}

void ModelPreCsRenderDemo::PreRender()
{
	sky->PreRender();
	if (true == bEditMode)
		actorEditor->PreRender();
}

void ModelPreCsRenderDemo::Render()
{
	if (true == bEditMode)
		actorEditor->Render();

	sky->Render(false);
	//gBuffer->Update();
	terrain->Render();
	static bool bTestPass = false;
	ImGui::Checkbox("TestPass", &bTestPass);

	for (Actor* actor : actors)
	{
		actor->Tech(1,1,1);
		actor->Pass(0,1,3);
		actor->Render();
	}
}

void ModelPreCsRenderDemo::PostRender()
{
}

void ModelPreCsRenderDemo::ResizeScreen()
{
}


void ModelPreCsRenderDemo::CreateBaseActor()
{
	Model* model = new Model(shader);
	model->ReadMaterial(L"Mutant/Mesh", L"../../_Textures/Model/");
	model->ReadMesh(L"Mutant/Mesh", L"../../_Models/");

	ModelAsset* modelasset = new ModelAsset(model);
	if (modelasset->HasAnim() != NULL)
	{

		modelasset->SetClip(L"Mutant/Mutant_Roaring", L"../../_Models/");
		modelasset->SetClip(L"Mutant/Mutant_Idle", L"../../_Models/");
	}
	ModelMeshComponent* modelMesh = modelasset->GetModelMeshCompFromModelAsset();

	Transform* transform = modelMesh->GetBaseInitTransform();
	transform->Position(0, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);

	Actor* actor = new Actor();
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);
	actors.emplace_back(actor);
	actorInstMap[actor] = 0;
	//Mutant
	model = new Model(shader);
	model->ReadMaterial(L"Kachujin/Mesh", L"../../_Textures/Model/");
	model->ReadMesh(L"Kachujin/Mesh", L"../../_Models/");

	modelasset = new ModelAsset(model);
	if (modelasset->HasAnim() != NULL)
	{
		modelasset->SetClip(L"Kachujin/S_M_H_Attack", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Idle", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Running", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Jump", L"../../_Models/");
	}
	modelMesh = modelasset->GetModelMeshCompFromModelAsset();

	transform = modelMesh->GetBaseInitTransform();
	transform->Position(0, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);

	actor = new Actor();
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);
	actors.emplace_back(actor);
	actorInstMap[actor] = 0;
}

void ModelPreCsRenderDemo::SetObjectTech(const UINT & mesh, const UINT & model, const UINT & anim)
{
}

void ModelPreCsRenderDemo::SetObjectPass(const UINT & mesh, const UINT & model, const UINT & anim)
{
}

void ModelPreCsRenderDemo::SceneRender()
{
}
