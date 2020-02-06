#include "stdafx.h"
#include "PBRDemo.h"
#include "Viewer/Freedom.h"
#include "Environment/Sky.h"
#include <iostream>

void PBRDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"027_Animation.fx");
	sky = new Sky(L"Environment/GrassCube1024.dds");

	brdfLut = new Texture(L"MaterialPBR/ibl_brdf_lut.png");
	Transform* transform = NULL;

	{
		floor = new Material(shader);
		floor->LoadDiffuseMap("Floor.png");
		floor->LoadNormalMap("Floor_Normal.png");
		floor->LoadSpecularMap("Floor_Specular.png");

		grid = new MeshRender(shader, new MeshGrid(5, 5));
		transform = grid->AddTransform();
		transform->Position(0, 0, 0);
		transform->Scale(12, 1, 12);
		grid->SetMaterial(floor);
		grid->UpdateTransforms();
	}
	{
		for (int i = 0; i < 9; i++)
			materials[i] = new MaterialPBR(shader);

		materials[0]->LoadAlbedoMap("AluminiumInsulator_Albedo.png");
		materials[0]->LoadNormalMap("AluminiumInsulator_Normal.png");
		materials[0]->LoadLoughnessMap("AluminiumInsulator_Roughness.png");
		materials[0]->LoadMetalicMap("AluminiumInsulator_Metallic.png");

		materials[1]->LoadAlbedoMap("CamoFabric_Albedo.png");
		materials[1]->LoadNormalMap("CamoFabric_Normal.png");
		materials[1]->LoadLoughnessMap("CamoFabric_Roughness.png");
		materials[1]->LoadMetalicMap("CamoFabric_Metallic.png");

		materials[2]->LoadAlbedoMap("GlassVisor_Albedo.png");
		materials[2]->LoadNormalMap("GlassVisor_Normal.png");
		materials[2]->LoadLoughnessMap("GlassVisor_Roughness.png");
		materials[2]->LoadMetalicMap("GlassVisor_Metallic.png");

		materials[3]->LoadAlbedoMap("Gold_Albedo.png");
		materials[3]->LoadNormalMap("Gold_Normal.png");
		materials[3]->LoadLoughnessMap("Gold_Roughness.png");
		materials[3]->LoadMetalicMap("Gold_Metallic.png");

		materials[4]->LoadAlbedoMap("GunMetal_Albedo.png");
		materials[4]->LoadNormalMap("GunMetal_Normal.png");
		materials[4]->LoadLoughnessMap("GunMetal_Roughness.png");
		materials[4]->LoadMetalicMap("GunMetal_Metallic.png");

		materials[5]->LoadAlbedoMap("IronOld_Albedo.png");
		materials[5]->LoadNormalMap("IronOld_Normal.png");
		materials[5]->LoadLoughnessMap("IronOld_Roughness.png");
		materials[5]->LoadMetalicMap("IronOld_Metallic.png");

		materials[6]->LoadAlbedoMap("Leather_Albedo.png");
		materials[6]->LoadNormalMap("Leather_Normal.png");
		materials[6]->LoadLoughnessMap("Leather_Roughness.png");
		materials[6]->LoadMetalicMap("Leather_Metallic.png");

		materials[7]->LoadAlbedoMap("SuperHeroFabric_Albedo.png");
		materials[7]->LoadNormalMap("SuperHeroFabric_Normal.png");
		materials[7]->LoadLoughnessMap("SuperHeroFabric_Roughness.png");
		materials[7]->LoadMetalicMap("SuperHeroFabric_Metallic.png");

		materials[8]->LoadAlbedoMap("Wood_Albedo.png");
		materials[8]->LoadNormalMap("Wood_Normal.png");
		materials[8]->LoadLoughnessMap("Wood_Roughness.png");
		materials[8]->LoadMetalicMap("Wood_Metallic.png");

		sphere = new MeshRender(shader, new MeshSphere(5));
		transform = sphere->AddTransform();
		transform->Position(0, 20, 0);
		sphere->UpdateTransforms();
		//for (int i = 0; i < 3; i++)
		//{
		//	for (int j = 0; j < 3; j++)
		//	{
		//		int index = i * 3 + j;
		//		sphere[index] = new MeshRender(shader, new MeshSphere(4));
		//		transform = sphere[index]->AddTransform();
		//		transform->Position(-10 * j + 20, 10 * i + 10, 0);
		//		/*Vector3 initpos;
		//		transform->Position(&initpos);
		//		cout << index << "_Sphere" << endl;
		//		cout << "<" << initpos.x
		//			<< "," << initpos.y
		//			<< "," << initpos.z
		//			<< ">" << endl;*/
		//		sphere[index]->SetMaterialPBR(materials[index]);
		//		sphere[index]->UpdateTransforms();
		//	}
		//}
	}

	shader->AsSRV("SkyCubeMap")->SetResource(sky->CubeSRV());
	shader->AsSRV("BRDFLUT")->SetResource(brdfLut->SRV());
}

void PBRDemo::Destroy()
{
}

void PBRDemo::Update()
{
	sky->Update();
	grid->Update();
	//for (int i = 0; i < 9; i++)
	{
		//sphere[i]->Update();
		sphere->Update();
	}

	static UINT selected = 0;
	ImGui::InputInt("MaterialPBR", (int*)&selected);
	selected %= 9;
	sphere->SetMaterialPBR(materials[selected]);
	sphere->GetMaterialPBR()->Property();
}

void PBRDemo::Render()
{
	sky->Render();

	grid->Render();
	//for (int i = 0; i < 9; i++)
	{
		sphere->Tech(1);
		sphere->Render();
	}
}
