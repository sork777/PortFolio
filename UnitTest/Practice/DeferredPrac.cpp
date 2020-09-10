#include "stdafx.h"
#include "DeferredPrac.h"
#include "PostEffects/HDR_Tone.h"
#include "Environment/Sky/Atmosphere.h"
#include "Environment/Ocean/FFTOcean.h"
#include "Environment/Terrain/TerrainLod.h"

void DeferredPrac::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader =SETSHADER(L"HW02_Deferred.fx");
	shadow = new CSM(shader);

	gBuffer = new GBuffer(shader);

	skyShader =SETSHADER(L"045_Scattering.fx");
	sky = new Atmosphere(skyShader);
	{
		terrainShader =SETSHADER(L"047_TerrainLod.fx");
		TerrainLod::InitializeInfo terraininfo =
		{
			shader,
			1.0f, 64, 20,
		};
		terrain = new TerrainLod(terraininfo);
		terrain->BaseTexture(L"Terrain/Dirt3.png");
		terrain->AlphaTexture(L"HeightMap/HeightMap512.png", true);
		//terrain->LayerTexture(L"Terrain/Grass (Lawn).jpg", L"HeightMap/HeightMap512.png");
		terrain->NDTexture(L"Terrain/Normal/Dirt3_Normal.png");// , L"Terrain/Displacement/Dirt3_Displacement.png");

		FFTOcean::InitializeInfo oceaninfo =
		{
			new Shader(L"HW07_OceanRendering.fx"),
			L"HeightMap/HeightMap512.png",
			1.0f, 64
		};
		ocean = new FFTOcean(oceaninfo);
		Vector3 pos;
		ocean->GetTransform()->Position(&pos);
		pos.y += 5.0f;
		ocean->GetTransform()->Position(pos);
	}
	hdr = new HDR_Tone();

	//CreateMesh
	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
		floor->LoadNormalMap("Floor_Normal.png");
		floor->LoadSpecularMap("Floor_Specular.png");

		stone = new Material(shader);
		stone->LoadDiffuseMap("Stones.png");
		stone->LoadNormalMap("Stones_Normal.png");
		stone->LoadSpecularMap("Stones_Specular.png");

		brick = new Material(shader);
		brick->LoadDiffuseMap("Bricks.png");
		brick->LoadNormalMap("Bricks_Normal.png");
		brick->LoadSpecularMap("Bricks_Specular.png");

		wall = new Material(shader);
		wall->LoadDiffuseMap("Wall.png");
		wall->LoadNormalMap("Wall_Normal.png");
		wall->LoadSpecularMap("Wall_Specular.png");

		Transform* transform;
		cube = new MeshRender(shader,new MeshCube());
		cube->SetMaterial(stone);
		cube->AddInstance();
		transform = cube->GetTransform(0);
		transform->Position(0, 2.5f, 0);
		transform->Scale(15.0f, 5.0f, 25.0);
		cube->UpdateTransforms();

		grid = new MeshRender(shader, new MeshGrid(10, 10));
		grid->SetMaterial(floor);
		grid->AddInstance();
		transform = grid->GetTransform(0);
		transform->Position(0, 0, 0);
		transform->Scale(20, 1, 20);
		grid->UpdateTransforms();


		cylinder = new MeshRender(shader,new MeshCylinder(0.5f, 3.0f, 20, 20));
		cylinder->SetMaterial(brick);
		sphere = new MeshRender(shader,new MeshSphere(0.5f, 20, 20));
		sphere->SetMaterial(wall);
		UINT cyCount = 0;
		UINT spCount = 0;
		for (UINT i = 0; i < 5; i++)
		{
			cylinder->AddInstance();
			transform = cylinder->GetTransform(cyCount++);
			transform->Position(-30, 6.0f, -30.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			cylinder->AddInstance();
			transform = cylinder->GetTransform(cyCount++);
			transform->Position(30, 6.0f, -30.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			sphere->AddInstance();
			transform = sphere->GetTransform(spCount++);
			transform->Position(-30, 15.5f, -30.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);

			sphere->AddInstance();
			transform = sphere->GetTransform(spCount++);
			transform->Position(30, 15.5f, -30.0f + (float)i * 15.0f);
			transform->Scale(5, 5, 5);
		}
		cylinder->UpdateTransforms();
		sphere->UpdateTransforms();
	}

	//Load Model
	{
		model = new Model(shader);
		model->ReadMaterial(L"B787/Airplane");
		model->ReadMesh(L"B787/Airplane");
		modelRender = new ModelRender(model);
		model->AddInstance();
		model->GetTransform(0)->Position(0,4.25f,0);
		model->GetTransform(0)->Scale(0.003f, 0.003f, 0.003f);
		model->UpdateTransforms();

		model = new Model(shader);
		model->ReadMaterial(L"Kachujin/Mesh");
		model->ReadMesh(L"Kachujin/Mesh");
		model->AddInstance();
		model->GetTransform(0)->Position(30, 0.0f, -40);
		model->GetTransform(0)->Scale(0.05f, 0.05f, 0.05f);
		model->UpdateTransforms();
		modelAnim = new ModelAnimator( model);
		modelAnim->ReadClip(L"Kachujin/Idle");
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
	ocean->Property();
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
	sphere->Update();
	cylinder->Update();

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
	terrain->Tech(1);
	terrain->Pass(0);
	terrain->Render();

	{
		sphere->Tech(0);
		sphere->Pass(meshPass);
		sphere->Render();
	}
	{
		cylinder->Tech(0);
		cylinder->Pass(meshPass);
		cylinder->Render();
	}
	cube->Tech(0);
	cube->Pass(meshPass);
	cube->Render();

	grid->Tech(0);
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Tech(0);
	modelRender->Pass(modelPass);
	modelRender->Render();
	modelAnim->GetModel()->Tech(0);
	modelAnim->GetModel()->Pass(animPass);
	modelAnim->Render();
}

void DeferredPrac::SetShadow(UINT tech)
{
	meshPass = 1;
	modelPass = 2;
	animPass = 3;

	{
		sphere->Tech(tech);
		sphere->Pass(meshPass);
		sphere->Render();
	}
	{
		cylinder->Tech(tech);
		cylinder->Pass(meshPass);
		cylinder->Render();
	}
	cube->Tech(tech);
	cube->Pass(meshPass);
	cube->Render();

	grid->Tech(tech);
	grid->Pass(meshPass);
	grid->Render();

	modelRender->Tech(tech);
	modelRender->Pass(modelPass);
	modelRender->Render();
	modelAnim->GetModel()->Tech(tech);
	modelAnim->Render();
}
