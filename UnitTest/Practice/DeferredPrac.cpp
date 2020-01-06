#include "stdafx.h"
#include "DeferredPrac.h"
#include "Objects/CSM.h"
#include "PostEffects/HDR_Tone.h"
#include "Environment/Atmosphere.h"
#include "Environment/Ocean/FFTOcean.h"
#include "Environment/TerrainLod.h"

void DeferredPrac::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"HW02_Deferred.fx");
	shadow = new CSM(shader);

	gBuffer = new GBuffer(shader);

	skyShader = new Shader(L"045_Scattering.fx");
	sky = new Atmosphere(skyShader);
	{
		terrainShader = new Shader(L"047_TerrainLod.fx");
		TerrainLod::InitializeInfo terraininfo =
		{
			shader,
			1.0f, 64, 20,
		};
		terrain = new TerrainLod(terraininfo);
		terrain->BaseTexture(L"Terrain/Dirt3.png");
		terrain->AlphaTexture(L"HeightMap/HeightMap512.png", true);
		//terrain->LayerTexture(L"Terrain/Grass (Lawn).jpg", L"HeightMap/HeightMap512.png");
		terrain->NDTexture(L"Terrain/Normal/Dirt3_Normal.png", L"Terrain/Displacement/Dirt3_Displacement.png");

		FFTOcean::InitializeInfo oceaninfo =
		{
			new Shader(L"HW07_OceanRendering.fx"),
			L"HeightMap/HeightMap512.png",
			1.0f, 64
		};
		ocean = new FFTOcean(oceaninfo);
	}
	hdr = new HDR_Tone();

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

		grid = new MeshGrid(shader, 10, 10);
		grid->GetTransform()->Position(0, 0, 0);
		grid->GetTransform()->Scale(20, 1, 20);


		for (UINT i = 0; i < 5; i++)
		{
			cylinder[i * 2] = new MeshCylinder(shader, 0.5f, 3.0f,false, 20, 20);
			//cylinder[i * 2]->GetTransform()->Position(-100, 6.0f, -50.0f + (float)i * 50.0f);
			cylinder[i * 2]->GetTransform()->Position(-30, 6.0f, -30.0f + (float)i * 15.0f);
			cylinder[i * 2]->GetTransform()->Scale(5, 5, 5);

			cylinder[i * 2 + 1] = new MeshCylinder(shader, 0.5f, 3.0f, false, 20, 20);
			//cylinder[i * 2 + 1]->GetTransform()->Position(100, 6.0f, -50.0f + (float)i * 50.0f);
			cylinder[i * 2 + 1]->GetTransform()->Position(30, 6.0f, -30.0f + (float)i * 15.0f);
			cylinder[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);


			sphere[i * 2] = new MeshSphere(shader, 0.5f, false, 20, 20);
			//sphere[i * 2]->GetTransform()->Position(-100.0f, 15.5f, -50.0f + i * 50.0f);
			sphere[i * 2]->GetTransform()->Position(-30, 15.5f, -30.0f + (float)i * 15.0f);
			sphere[i * 2]->GetTransform()->Scale(5, 5, 5);

			sphere[i * 2 + 1] = new MeshSphere(shader, 0.5f, false, 20, 20);
			//sphere[i * 2 + 1]->GetTransform()->Position(100.0f, 15.5f, -50.0f + i * 50.0f);
			sphere[i * 2 + 1]->GetTransform()->Position(30, 15.5f, -30.0f + (float)i * 15.0f);
			sphere[i * 2 + 1]->GetTransform()->Scale(5, 5, 5);
		}
	}

	//Load Model
	{
		model = new Model();
		model->ReadMaterial(L"eclipse/eclipse");
		model->ReadMesh(L"eclipse/eclipse");
		modelRender = new ModelRender(shader, model);
		modelRender->AddTransform();
		modelRender->GetTransform(0)->Position(0,4.25f,0);
		modelRender->GetTransform(0)->Scale(0.3f, 0.3f, 0.3f);
		modelRender->UpdateTransform();

		model = new Model();
		model->ReadMaterial(L"Kachujin/Kachujin");
		model->ReadMesh(L"Kachujin/Kachujin");
		model->ReadClip(L"Kachujin/Ninja_Idle");
		modelAnim = new ModelAnimator(shader, model);
		modelAnim->AddTransform();
		modelAnim->GetTransform(0)->Position(30, 0.0f, -40);
		modelAnim->GetTransform(0)->Scale(0.05f, 0.05f, 0.05f);
		modelAnim->UpdateTransform();
	}
	//PointLights
	{
		pointLights = new Lights::PointLight(shader);
		Lights::PointLightInfo light;
		
		for (UINT i = 0; i < 1; i++)
		{
			light.color = Math::RandomColor3();
			light.Position = Vector3(i*30.0f-30.0f, 25, 30.0f);
			light.Range = 40.0f;
			light.Specular = 30.0f;
			pointLights->AddLight(light);
		}		
		int a = 0;
	}
	//SpotLights
	{
		spotLights = new Lights::SpotLight(shader);
		Lights::SpotLightInfo light;

		for (UINT i = 0; i < 1; i++)
		{
			light.color = Math::RandomColor3();
			light.Position = Vector3(i*30.0f - 30.0f, 25, -30.0f);
			light.Direction = Vector3(0, -1, 0);
			light.InnerAngle = 30.0f;
			light.OuterAngle = 45.0f;
			light.Range = 40.0f;
			light.Specular = 15.0f;

			spotLights->AddLight(light);
			
		}
		int a = 0;
	}

	//CapsuleLights
	{
		capLights = new Lights::CapsuleLight(shader);
		Lights::CapsuleLightInfo light;

		for (UINT i = 0; i < 1; i++)
		{
			light.color = Math::RandomColor3();
			light.Position = Vector3(i*30.0f - 30.0f, 2, -80.0f);
			light.Direction = Math::RandomVec3(0,1);
			light.Direction.y = 0;
			D3DXVec3Normalize(&light.Direction, &light.Direction);
			light.Length = 50.0f;
			light.Range = 10.0f;
			light.Specular = 10.0f;
			capLights->AddLight(light);
		}
		int a = 0;
	}

	
	ssao = new SSAO();
	sslr = new SSLR();

}



void DeferredPrac::Update()
{
	app_time += Time::Delta();
	ocean->Update(app_time);

	sky->Update();
	terrain->Update();
	terrain->TerrainController();

	gBuffer->Update();
	
	modelAnim->PlayAnim();
	/* ±¤¿ø Á¶Á¤*/
	bool bDocking = true;
	ImGui::Begin("Lights Controller", &bDocking);
	{
		pointLights->LightController();
		spotLights->LightController();
		capLights->LightController();
		ImGui::End();
	}
	ImGui::Begin("SSAO Controller", &bDocking);
	{
		static float radius = 25.0f;
		static int SSAOSampleRadius = 10;
		static bool bAO = true;
		ImGui::SliderFloat("Radius",&radius , 10, 100);
		ImGui::SliderInt("SSAOSampleRadius",&SSAOSampleRadius, 5, 30);
		ImGui::Checkbox("UseAO", &bAO);
		shader->AsScalar("UseAO")->SetInt(bAO?1:0);

		ssao->SetParameters(SSAOSampleRadius, radius);
		ImGui::End();
	}
	ImGui::Begin("HDR Controller", &bDocking);
	{
		static float middlegrey = 40.863f;
		static float white = 41.53f;
		ImGui::SliderFloat("MiddleGray", &middlegrey, 0.10f, 50.0f);
		ImGui::SliderFloat("White", &white, 0.10f, 50.0f);

		hdr->SetParameters(middlegrey, white);
		ImGui::End();
	}
	ImGui::Begin("SSLR Controller", &bDocking);
	{
		
		ImGui::ColorEdit3("LightRayColor", (float*)&Context::Get()->LightSpecular());
		ImGui::SliderFloat("LightRayIntensity", &Context::Get()->LightSpecular().a,0.01f,10.0f);

		ImGui::End();
	}


	/* ¿ÀºêÁ§Æ® ¾÷µ¥ÀÌÆ® */
	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Update();
		cylinder[i]->Update();
	}

	cube->Update();
	grid->Update();
	modelRender->Update();
	modelAnim->Update();
}

void DeferredPrac::Render()
{
	hdr->Set(gBuffer->GetDepthOnlyDsv());
	gBuffer->Tech(1);
	gBuffer->Render();

	pointLights->Tech(2);
	pointLights->Render();

	spotLights->Tech(3);
	//spotLights->Pass(4);
	spotLights->Render();

	capLights->Tech(4);
	capLights->Render();
	hdr->Tech(0);
	hdr->PostProcessing(gBuffer->GetDepthDsv());
	ocean->Render(app_time);
}

void DeferredPrac::PreRender()
{
	sky->PreRender();

	///////////////////////////////////////////////////////
	/* pl_shadow ¸Ê Âï±â */
	bool bDraw = true;
	while (bDraw == true)
	{
		//RTV
		bDraw = pointLights->LightPreSet();
		SetShadow(2);
	}
	///////////////////////////////////////////////////////

	bDraw = true;
	while (bDraw == true)
	{
		bDraw = spotLights->LightPreSet();
		SetShadow(3);
	}
	///////////////////////////////////////////////////////
	//* CSM shadow ¸Ê Âï±â */
	shadow->Set();
	SetShadow(1);
	///////////////////////////////////////////////////////

	SetGBuffer();
	//sky
	sky->Render();
	shader->AsSRV("AtmosphereMap")->SetResource(sky->GetAtmoSRV());
	//SSAO
	ssao->Compute(gBuffer->GetDepthSrv(), gBuffer->GetNormalSrv());
	shader->AsSRV("AOTexture")->SetResource(ssao->GetSSAOSRV());
	

}

void DeferredPrac::PostRender()
{
	//sky->PostRender();
	ocean->Render(app_time);
	sslr->Render(ssao->GetSSAOSRV());


	gBuffer->RenderGBuffers();
	ssao->RenderSSAO();
	ocean->RenderFFT();
}

void DeferredPrac::SetGBuffer()
{
	///////////////////////////////////////////////////////
	/* ºÐ¸® */
	gBuffer->SetRTVs();
	
	meshPass = 0;
	modelPass = 1;
	animPass = 2;
	//terrainPass = 3; 
	terrain->Pass(3);
	terrain->Render();

	

	wall->Render();

	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Tech(0);
		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < 10; i++)
	{
		cylinder[i]->Tech(0);
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}
	stone->Render();
	cube->Tech(0);
	cube->Pass(meshPass);
	cube->Render();
	floor->Render();

	grid->Tech(0);
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Tech(0);
	modelRender->Pass(modelPass);
	modelRender->Render();
	modelAnim->Tech(0);
	modelAnim->Pass(animPass);
	modelAnim->Render();
}

void DeferredPrac::SetShadow(UINT tech)
{
	meshPass = 1;
	modelPass = 2;
	animPass = 3;

	wall->Render();

	for (UINT i = 0; i < 10; i++)
	{
		sphere[i]->Tech(tech);
		sphere[i]->Pass(meshPass);
		sphere[i]->Render();
	}
	brick->Render();
	for (UINT i = 0; i < 10; i++)
	{
		cylinder[i]->Tech(tech);
		cylinder[i]->Pass(meshPass);
		cylinder[i]->Render();
	}
	stone->Render();
	cube->Tech(tech);
	cube->Pass(meshPass);
	cube->Render();

	floor->Render();
	grid->Tech(tech);
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Tech(tech);
	modelRender->Pass(modelPass);
	modelRender->Render();
	modelAnim->Tech(tech);
	modelAnim->Render();
}
