#include "stdafx.h"
#include "TerrainLodDemo.h"
#include "Objects/CSM.h"
#include "Environment/Atmosphere.h"
#include "Environment/TerrainLod.h"

void TerrainLodDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"HW02_Deferred.fx");
	shadow = new CSM(shader);
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
	terrainMat = new Material(shader);

	ssao = new SSAO();

}

void TerrainLodDemo::Destroy()
{
	SafeDelete(frustum);

}

void TerrainLodDemo::Update()
{
	sky->Update();
	terrain->TerrainController();
	terrain->Update();

	gBuffer->Update();

}

void TerrainLodDemo::PreRender()
{
	sky->PreRender();
	sky->Render();
	//shadow->Set();
	
	SetGBuffer();
	shader->AsSRV("AtmosphereMap")->SetResource(sky->GetAtmoSRV());
	ssao->Compute(gBuffer->GetDepthSrv(), gBuffer->GetNormalSrv());
	shader->AsSRV("AOTexture")->SetResource(ssao->GetSSAOSRV());
}


void TerrainLodDemo::Render()
{

	gBuffer->Tech(1);
	gBuffer->Render();
	
	gBuffer->RenderGBuffers();
	ssao->RenderSSAO();
}

void TerrainLodDemo::SetGBuffer()
{
	///////////////////////////////////////////////////////
	/* 분리 */
	gBuffer->SetRTVs();
	
	terrainMat->Render();
	terrain->Tech(1);
	terrain->Render();

}
