#include "stdafx.h"
#include "LevelEditorDemo.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Terrain/TerrainLod.h"

#include "Editor/TerrainEditor.h"
#include "Objects/Manager/ActorManager.h"


void LevelEditorDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);
	//shader = SETSHADER(L"027_Animation.fx");
	shader = SETSHADER(L"HW02_Deferred.fx");
	//Deffered Cascade shadow
	{
		shadow = new CSM(shader);
		gBuffer = new GBuffer(shader);
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

	ssao = new SSAO();
	actorMgr = new ActorManager();
	actorMgr->SetTerrain(terrain);

	//PBR관련
	cubeTex = new TextureCube((Vector3&)Vector3(0, 0, 0), 512, 512);
	Texture* brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	shader->AsSRV("SkyCubeMap")->SetResource(cubeTex->SRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());

	//frustum
	perspective = new Perspective(D3D::Width(), D3D::Height(), 1.0f, 1000.0f, Math::PI * 0.25f);
	frustum = new Frustum(NULL, perspective);

	CreateBaseActor();
}

void LevelEditorDemo::Destroy()
{
	SafeDelete(frustum);
	SafeDelete(cubeTex);
	SafeDelete(ssao);
	SafeDelete(actorMgr);
	SafeDelete(terrainEditor);
	SafeDelete(terrain);
	SafeDelete(frustum);
	SafeDelete(perspective);
}

void LevelEditorDemo::Update()
{
	sky->Update();
	gBuffer->Update();
	terrain->Update();

	if (true == bEditMode)
		EditModeUpdate();
	else
		PlayModeUpdate();

	actorMgr->SetSpawnPosition(terrain->GetPickedPosition());
	actorMgr->Update();
	if (true == bActorSpwan)
		actorMgr->ObjectSpawn();
}

void LevelEditorDemo::PreRender()
{
	if (true == bEditMode)
		terrainEditor->PreRender();
	else
		terrain->PreRender();

	sky->PreRender();
	//Shadow 필요
	{

	}
	bOpenActorEditor = actorMgr->IsEditorOpened();
	bActorSpwan = actorMgr->IsObjectSpawn();

	if (true == bEditMode)
		EditModePreRender();
	else
		PlayModePreRender();
	actorMgr->PreRender();

	sky->Render();
	shader->AsSRV("AtmosphereMap")->SetResource(sky->GetAtmoSRV());
	ssao->Compute(gBuffer->GetDepthSrv(), gBuffer->GetNormalSrv());
	shader->AsSRV("AOTexture")->SetResource(ssao->GetSSAOSRV());
	cubeTex->Set(skyShader);
	sky->Render(false);

}

void LevelEditorDemo::Render()
{
	if (true == bEditMode)
		EditModeRender();
	else
		PlayModeRender();

	static bool bCreBu = false;
	if (bCreBu == false)
	{
		for (ContentsAsset* asset : AssetManager::Get()->GetAllAssets())
		{
			asset->CreateButtonImage();
		}
		//actorMgr->CreatActorButtonImage(actor);

		bCreBu = true;
	}
}

void LevelEditorDemo::CreateBaseActor()
{
	//Kachujin
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

	Actor* actor = actorMgr->RegistActor(new Actor());
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);

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

	actor = actorMgr->RegistActor(new Actor());
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);


	//Sword
	model = new Model(shader);
	model->ReadMaterial(L"Weapon/Sword", L"../../_Textures/Model/");
	model->ReadMesh(L"Weapon/Sword", L"../../_Models/");

	modelasset = new ModelAsset(model);
	modelMesh = modelasset->GetModelMeshCompFromModelAsset();
	
	transform = modelMesh->GetBaseInitTransform();
	transform->RotationDegree(0, 0, 90);
	transform->Position(-10, -5, -15);
}

void LevelEditorDemo::SetObjectTech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	actorMgr->Tech(mesh, model, anim);
}

void LevelEditorDemo::SetObjectPass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	actorMgr->Pass(mesh, model, anim);
}

void LevelEditorDemo::SceneRender()
{
	actorMgr->Render();
}

void LevelEditorDemo::PlayModeUpdate()
{
	//모드 전환 영역 
	if(Keyboard::Get()->Down(VK_ESCAPE))
		bEditMode = true;

	terrain->Update();

}

void LevelEditorDemo::EditModeUpdate()
{
	//온갖 설정 다 넣을거임
	Context::Get()->ShowContextInfo();
	//모드 전환 영역 
	ImGui::Begin("##Play", &bEditMode);
	{
		if (ImGui::Button("TurnPlayMode", ImVec2(120, 40)))
			bEditMode = false;
		ImGui::End();
	}

	sky->Property();
	terrainEditor->TerrainController();
	ImGui::Begin("SSAO Controller", &bEditMode);
	{
		static float radius = 5.0f;
		static int SSAOSampleRadius = 5;
		static bool bAO = true;
		ImGui::SliderFloat("Radius", &radius, 1, 10);
		ImGui::SliderInt("SSAOSampleRadius", &SSAOSampleRadius, 3, 10);
		ImGui::Checkbox("UseAO", &bAO);
		shader->AsScalar("UseAO")->SetInt(bAO ? 1 : 0);

		ssao->SetParameters(SSAOSampleRadius, radius);
		ImGui::End();
	}
	if (true == bOpenActorEditor)
		actorMgr->UpdateActorEditor();

}

void LevelEditorDemo::PlayModePreRender()
{
	//GBuffer
	{
		gBuffer->SetRTVs();

		terrainMat->Render();
		terrain->Tech(1);
		terrain->Render();

		SetObjectTech(0, 0, 0);
		SetObjectPass(0, 1, 2);
		SceneRender();
	}
}

void LevelEditorDemo::EditModePreRender()
{

	//GBuffer
	{
		gBuffer->SetRTVs();

		terrainMat->Render();
		terrain->Tech(1);
		terrainEditor->Render();

		SetObjectTech(0, 0, 0);
		SetObjectPass(0, 1, 3);
		SceneRender();
	}
	
	if (true == bOpenActorEditor)
		actorMgr->PreRenderActorEditor();
}

void LevelEditorDemo::PlayModeRender()
{
	gBuffer->Tech(1);
	gBuffer->Render();
}

void LevelEditorDemo::EditModeRender()
{
	actorMgr->ObjectIcon();
	if (true == bOpenActorEditor)
		actorMgr->RenderActorEditor();
	

	gBuffer->Tech(1);
	gBuffer->Render();

	gBuffer->RenderGBuffers();
	ssao->RenderSSAO();
	AssetManager::Get()->ShowAssets();
}
