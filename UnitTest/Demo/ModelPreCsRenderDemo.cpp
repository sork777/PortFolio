#include "stdafx.h"
#include "ModelPreCsRenderDemo.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Terrain/TerrainLod.h"

#include "Editor/TerrainEditor.h"
#include "Objects/Manager/ActorManager.h"


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
	actorMgr = new ActorManager();
	actorMgr->SetTerrain(terrain);
	//ssao = new SSAO();
	//CreateBaseModels();
	CreateBaseActor();
	//PBR°ü·Ã
	cubeTex = new TextureCube((Vector3&)Vector3(0, 0, 0), 512, 512);
	Texture* brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	shader->AsSRV("SkyCubeMap")->SetResource(cubeTex->SRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());



	float width = D3D::Width();
	float height = D3D::Height();

	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);
	render2D = new Render2D();
	render2D->GetTransform()->Position(width*0.5f,height*0.5f, 0);
	render2D->GetTransform()->Scale(width, height, 1);


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

	if (true == bEditMode)
		actorMgr->UpdateActorEditor();
	actorMgr->SetSpawnPosition(terrain->GetPickedPosition());
	actorMgr->Update();
	if (true == bActorSpwan)
		actorMgr->ObjectSpawn();
}

void ModelPreCsRenderDemo::PreRender()
{
	terrain->PreRender();
	sky->PreRender();

	bEditMode = actorMgr->IsEditorOpened();
	bActorSpwan = actorMgr->IsObjectSpawn();

	renderTarget->Set(depthStencil->DSV());
	{

		sky->Render(false);
		//gBuffer->Update();
		terrain->Render();
		actorMgr->Tech(1, 1, 1);
		actorMgr->Pass(0, 1, 3);
		actorMgr->Render();
	}
	if (true == bEditMode)
		actorMgr->PreRenderActorEditor();

	actorMgr->PreRender();
}

void ModelPreCsRenderDemo::Render()
{
	actorMgr->ObjectIcon();
	if (true == bEditMode)
		actorMgr->RenderActorEditor();

	render2D->SRV(renderTarget->SRV());
	render2D->Render();
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

	//Actor* actor = new Actor();
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

	actor = actorMgr->RegistActor(new Actor());;
	//actor = new Actor();
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);
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
