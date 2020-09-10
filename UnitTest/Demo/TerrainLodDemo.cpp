#include "stdafx.h"
#include "TerrainLodDemo.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Terrain/TerrainLod.h"
#include "Editor/TerrainEditor.h"

#include "Objects/Manager/ActorManager.h"

void TerrainLodDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = SETSHADER(L"HW02_Deferred.fx");
	//shader = SETSHADER(L"027_Animation.fx");
	shadow = new CSM(shader);
	gBuffer = new GBuffer(shader);
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
			//shader,
			1.0f, 64, 40,
		};
		terrain = new TerrainLod(info);
		terrain->SetDebugMode(true);
		terrain->SetDefferedMode(true);
		tEdit = new TerrainEditor(terrain);

		camera = new Fixity();
		camera->Position(0, 0, -150);
		perspective = new Perspective(D3D::Width(), D3D::Height(), 1.0f, 1000.0f, Math::PI * 0.25f);

		frustum = new Frustum(camera, perspective);
	}
	//디퍼드를 위한 터레인용 마테리얼
	terrainMat = new Material(terrainShader);
	
	ssao = new SSAO();
	actorMgr = new ActorManager();
	actorMgr->SetTerrain(terrain);

	CreateBaseActor();
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
		floor->LoadNormalMap("Floor_Normal.png");
		//floor->LoadSpecularMap("Floor_Specular.png");
		floor->LoadHeightMap("Floor_Displacement.png");
	}

	//Create Mesh
	{
		Transform* transform = NULL;
		grid = new MeshRender(shader, new MeshCube());
		grid->SetMaterial(floor);
		grid->AddInstance();
		transform = grid->GetTransform(0);
		transform->Position(0, 1.0f, 0);
		transform->Scale(12, 2, 12);
		grid->AddInstance();
		transform = grid->GetTransform(1);
		transform->Position(0, 10, 0);
		transform->Scale(6, 18, 6);
	}
	grid->UpdateTransforms();

	cubeTex = new TextureCube((Vector3&)Vector3(0, 0, 0), 512, 512);
	Texture* brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");	
	shader->AsSRV("SkyCubeMap")->SetResource(cubeTex->SRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());

}

void TerrainLodDemo::Destroy()
{
	SafeDelete(frustum);

}

void TerrainLodDemo::Update()
{
	
	sky->Update();	
	gBuffer->Update();
	//terrain->TerrainController();
	/*Vector3 pos = Vector3(0, 30, 0);
	cubeTex->Position(pos);
*/
	grid->Update();

	static bool bTerrainEdit = false;
	if (ImGui::Checkbox("TerrainEdit", &bTerrainEdit))
		terrain->SetEditMode(bTerrainEdit);

	if (true == terrain->GetEditMode())
	{
		tEdit->Update();
		tEdit->TerrainController();
	}
	else
		terrain->Update();

	actorMgr->SetSpawnPosition(terrain->GetPickedPosition());
	actorMgr->Update();
}

void TerrainLodDemo::PreRender()
{
	if (true == terrain->GetEditMode())
		tEdit->PreRender();
	else
		terrain->PreRender();
	
	
	sky->PreRender();
	{
		//TODO:0902 왠지 디퍼드 그림자가 안먹는다...?
		shadow->Set();
		grid->Tech(1);
		grid->Pass(1);
		floor->Render();
		grid->Render();

		if (NULL != actor)
		{
			actorMgr->Tech(1, 1, 1);
			actorMgr->Pass(1, 2, 3);
			actorMgr->Render();
		}
	}
	SetGBuffer();

	sky->Render();
	shader->AsSRV("AtmosphereMap")->SetResource(sky->GetAtmoSRV());
	ssao->Compute(gBuffer->GetDepthSrv(), gBuffer->GetNormalSrv());
	shader->AsSRV("AOTexture")->SetResource(ssao->GetSSAOSRV());
	cubeTex->Set(skyShader);
	sky->Render(false);
	/*
	if (true == terrain->GetEditMode())
		tEdit->Render();
	else
		terrain->Render();*/
	actorMgr->PreRender();
}


void TerrainLodDemo::Render()
{
	static bool bCreBu = false;

	//ImGui::Checkbox("AssetButton", &bCreBu);
	
	
	bool bDocking = true;
	ImGui::Begin("SSAO Controller", &bDocking);
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
	actorMgr->ObjectIcon();
	

	gBuffer->Tech(1);
	gBuffer->Render();
	
	gBuffer->RenderGBuffers();
	ssao->RenderSSAO();
	AssetManager::Get()->ShowAssets();
	
	//마지막에 안하면 다른애랑 같이 찍힘
	if (bCreBu == false)
	{
		for (ContentsAsset* asset : AssetManager::Get()->GetAllAssets())
		{
			asset->CreateButtonImage();
		}

		bCreBu = true;
	}
}

void TerrainLodDemo::SetGBuffer()
{
	///////////////////////////////////////////////////////
	/* 분리 */
	gBuffer->SetRTVs();
	
	terrainMat->Render();
	terrain->Tech(1);
	static bool bWire = false;
	ImGui::Checkbox("WireFrame", &bWire);
	terrain->Pass(bWire);
	if (true == terrain->GetEditMode())
		tEdit->Render();
	else
		terrain->Render();
	//terrain->Render();
	grid->Tech(0);
	grid->Pass(0);
	floor->Render();
	grid->Render();

	{
		actorMgr->Tech(0, 0, 0);
		actorMgr->Pass(0, 1, 2);
		actorMgr->Render();		
	}	
}

void TerrainLodDemo::CreateBaseActor()
{
	Model* model = new Model(shader);
	model->ReadMaterial(L"Kachujin/Mesh", L"../../_Textures/Model/");
	model->ReadMesh(L"Kachujin/Mesh", L"../../_Models/");
	
	ModelAsset* modelasset = new ModelAsset(model);
	// 모델에셋에서 애니메이터 클립 정보도 같이 공유
	if(modelasset->HasAnim() != NULL)
	{
		//modelasset->SetClip(L"Kachujin/Mesh", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/S_M_H_Attack", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Idle", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Running", L"../../_Models/");
		modelasset->SetClip(L"Kachujin/Jump", L"../../_Models/");
	}

	ModelMeshComponent* modelMesh = modelasset->GetModelMeshCompFromModelAsset();
	
	Transform* transform = modelMesh->GetBaseInitTransform();
	transform->Position(0, 0, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);

	actor = actorMgr->RegistActor( new Actor());
	actor->SetRootComponent(modelMesh);
	actor->SetShader(shader);

	model = new Model(shader);
	model->ReadMaterial(L"Weapon/Sword", L"../../_Textures/Model/");
	model->ReadMesh(L"Weapon/Sword", L"../../_Models/");
	
	modelasset = new ModelAsset(model);
	modelMesh = modelasset->GetModelMeshCompFromModelAsset();
	transform = modelMesh->GetBaseInitTransform();

	transform->RotationDegree(0, 0, 90);
	transform->Position(-10, -5, -15);

	modelMesh->AttachSocket(L"RightHand");
	actor->GetRootMeshData()->LinkChildComponent(modelMesh);
	
	selecedComp = actor->GetRootMeshData();

}