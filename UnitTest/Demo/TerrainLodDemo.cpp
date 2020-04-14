#include "stdafx.h"
#include "TerrainLodDemo.h"
#include "Objects/Actor/Actor.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/TerrainLod.h"

void TerrainLodDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	//shader = new Shader(L"HW02_Deferred.fx");
	shader = new Shader(L"027_Animation.fx");
	//shadow = new CSM(shader);
	gBuffer = new GBuffer(shader);
	//Sky
	{
		skyShader = new Shader(L"045_Scattering.fx");
		sky = new Atmosphere(skyShader);

	}
	//Terrain
	{
		terrainShader = new Shader(L"047_TerrainLod.fx");

		TerrainLod::InitializeInfo info =
		{
			terrainShader,
			//shader,
			1.0f, 64, 20,
		};
		terrain = new TerrainLod(info);
		//terrain->BaseTexture(L"Terrain/Dirt3.png");
		//terrain->NDTexture(L"Terrain/Normal/Dirt3_Normal.png");
		camera = new Fixity();
		camera->Position(0, 0, -150);
		perspective = new Perspective(D3D::Width(), D3D::Height(), 1.0f, 1000.0f, Math::PI * 0.25f);

		//NULL : 기본 카메라 쓸거임
		//frustum = new Frustum(NULL, perspective);
		frustum = new Frustum(camera, perspective);
	}
	//디퍼드를 위한 터레인용 마테리얼
	terrainMat = new Material(terrainShader);
	//terrainMat->Specular(1, 1, 1, 1);
	
	ssao = new SSAO();
		
	CreateBaseActor();

	cubeTex = new TextureCube((Vector3&)Vector3(0, 0, 0), 256, 256);
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
	//gBuffer->Update();
	terrain->TerrainController();
	terrain->Update();
	/*Vector3 pos = Vector3(0, 30, 0);
	cubeTex->Position(pos);
*/
	if (NULL != actor)
	{
		static bool bSpwan = false;
		actor->Update();

		if (true == bSpwan )
		{
			actor->SetSpawnPosition(terrain->GetPickedPosition());
			if(Mouse::Get()->Down(0))
				bSpwan = false;
		}


		if (ImGui::Button("TestSpwanActor"))
		{
			actor->AddInstanceData();
			bSpwan = true;
		}

		ImGui::Separator();
		actor->ShowCompHeirarchy(&selecedComp);
		if (selecedComp != NULL)
			selecedComp->Property();		
	}

}

void TerrainLodDemo::PreRender()
{
	/*for (ContentsAsset* asset : AssetManager::Get()->GetAllAssets())
	{
		asset->CreateButtonImage();
	}
*/
	sky->PreRender();
	{
	/*	shadow->Set();
		if (NULL != actor)
		{
			ObjectBaseComponent* root = actor->GetRootMeshData();
			root->Tech(1, 1, 1);
			root->Pass(1, 2, 3);
			actor->Render();
		}*/
	}
	{
	//	cubeTex->Set(shader);
	//	sky->Render(false);		
	//	shader->AsSRV("SkyCubeMap")->SetResource(cubeTex->SRV());
	}
	//SetGBuffer();

	//shader->AsSRV("AtmosphereMap")->SetResource(sky->GetAtmoSRV());
	//ssao->Compute(gBuffer->GetDepthSrv(), gBuffer->GetNormalSrv());
	//shader->AsSRV("AOTexture")->SetResource(ssao->GetSSAOSRV());
	
}


void TerrainLodDemo::Render()
{
	sky->Render(false);

	//terrainMat->Render();
	terrain->Render();

	//gBuffer->Tech(1);
	//gBuffer->Render();
	//
	//gBuffer->RenderGBuffers();
	//ssao->RenderSSAO();
	//AssetManager::Get()->ShowAssets();

	if (NULL != actor)
	{
		ObjectBaseComponent* root = actor->GetRootMeshData();
		root->Tech(1, 1, 1);
		root->Pass(0, 1, 2);
		actor->Render();
	}
}

void TerrainLodDemo::SetGBuffer()
{
	///////////////////////////////////////////////////////
	/* 분리 */
	gBuffer->SetRTVs();
	
	terrainMat->Render();
	terrain->Tech(1);
	//terrain->Pass(3);
	terrain->Render();

	//if (NULL != actor)
	//{
	//	ObjectBaseComponent* root = actor->GetRootMeshData();
	//	root->Tech(0, 0, 0);
	//	root->Pass(0, 1, 2);
	//	actor->Render();
	//}

}

void TerrainLodDemo::CreateBaseActor()
{
	Texture* testTex =new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	TextureAsset* textasset = new TextureAsset(testTex);

	Model* model = new Model(shader);
	model->ReadMaterial(L"Kachujin/Mesh", L"../../_Textures/Model/");
	model->ReadMesh(L"Kachujin/Mesh", L"../../_Models/");
	
	ModelAsset* modelasset = new ModelAsset(model);
	ModelMeshComponent* modelMesh = modelasset->GetModelMeshCompFromModelAsset();
	
	Transform* transform = modelMesh->GetBaseTransform();
	transform->Position(0, 5, 0);
	transform->Scale(0.075f, 0.075f, 0.075f);

	ModelAnimator* animator = modelMesh->GetAnimation();
	if(animator != NULL)
	{
		animator->ReadClip(L"Kachujin/S_M_H_Attack", L"../../_Models/");
		animator->ReadClip(L"Kachujin/Mesh", L"../../_Models/");
		animator->ReadClip(L"Kachujin/Idle", L"../../_Models/");
		animator->ReadClip(L"Kachujin/Running", L"../../_Models/");
		animator->ReadClip(L"Kachujin/Jump", L"../../_Models/");
	}

	actor = new Actor(modelMesh);
	actor->GetRootMeshData()->SetShader(shader);

	model = new Model(shader);
	model->ReadMaterial(L"Weapon/Sword", L"../../_Textures/Model/");
	model->ReadMesh(L"Weapon/Sword", L"../../_Models/");
	
	modelasset = new ModelAsset(model);
	modelMesh = modelasset->GetModelMeshCompFromModelAsset();
	transform = modelMesh->GetBaseTransform();

	transform->RotationDegree(0, 0, 90);
	transform->Position(-10, -5, -15);

	//modelMesh->AttachSocket(L"RightHand");
	actor->GetRootMeshData()->LinkChildComponent(modelMesh);

	selecedComp = actor->GetRootMeshData();
}