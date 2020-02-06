#include "stdafx.h"
#include "Convert.h"
//#include "Converter/Converter.h"

void Convert::Initialize()
{
	//Airplane();
	//Tower();
	//Tank();
	//Kachujin();
	//Megan();
	//Weapon();
	//Mutant();
	Temp();
}

void Convert::Airplane()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"B787/Airplane.fbx");
	conv->ExportMaterial(L"B787/Airplane", L"../../_Textures/");
	conv->ExportMesh(L"B787/Airplane");

	SafeDelete(conv);
}

void Convert::Tower()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tower/Tower.fbx");
	conv->ExportMaterial(L"Tower/Tower", L"../../_Textures/");
	conv->ExportMesh(L"Tower/Tower");

	SafeDelete(conv);
}

void Convert::Tank()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Tank/Tank.fbx");
	conv->ExportMaterial(L"Tank/Tank", L"../../_Textures/");
	conv->ExportMesh(L"Tank/Tank");

	SafeDelete(conv);
}

void Convert::Kachujin()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Kachujin/Mesh.fbx");
	conv->ExportMaterial(L"Kachujin/Mesh", L"../../_Textures/");
	conv->ExportMesh(L"Kachujin/Mesh", L"../../_Models/");
	conv->ExportAnimClip(0, L"Kachujin/Mesh", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Idle.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Idle", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Running.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Running", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Jump.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Jump", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Kachujin/Hip_Hop_Dancing.fbx");
	conv->ExportAnimClip(0, L"Kachujin/Hip_Hop_Dancing", L"../../_Models/");
	SafeDelete(conv);
}

void Convert::Megan()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Megan/Mesh.fbx");
	conv->ExportMaterial(L"Megan/Mesh", L"../../_Textures/");
	conv->ExportMesh(L"Megan/Mesh", L"../../_Models/");
	conv->ExportAnimClip(0, L"Megan/Mesh", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Megan/Taunt.fbx");
	conv->ExportAnimClip(0, L"Megan/Taunt", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Megan/Dancing.fbx");
	conv->ExportAnimClip(0, L"Megan/Dancing", L"../../_Models/");
	SafeDelete(conv);
}

void Convert::Mutant()
{
	Converter* conv = new Converter();
	conv->ReadFile(L"Mutant/mutant.fbx");
	conv->ExportMaterial(L"Mutant/Mesh", L"../../_Textures/");
	conv->ExportMesh(L"Mutant/Mesh", L"../../_Models/");
	conv->ExportAnimClip(0, L"Mutant/Mesh", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Mutant/Mutant_Roaring.fbx");
	conv->ExportAnimClip(0, L"Mutant/Mutant_Roaring", L"../../_Models/");
	SafeDelete(conv);

	conv = new Converter();
	conv->ReadFile(L"Mutant/Mutant_Idle.fbx");
	conv->ExportAnimClip(0, L"Mutant/Mutant_Idle", L"../../_Models/");
	SafeDelete(conv);
}

void Convert::Weapon()
{
	vector<wstring> names;
	names.push_back(L"Cutter.fbx");
	names.push_back(L"Cutter2.fbx");
	names.push_back(L"Dagger_epic.fbx");
	names.push_back(L"Dagger_small.fbx");
	names.push_back(L"Katana.fbx");
	names.push_back(L"LongArrow.obj");
	names.push_back(L"LongBow.obj");
	names.push_back(L"Rapier.fbx");
	names.push_back(L"Sword.fbx");
	names.push_back(L"Sword2.fbx");
	names.push_back(L"Sword_epic.fbx");

	for (wstring name : names)
	{
		Converter* conv = new Converter();
		conv->ReadFile(L"Weapon/" + name);


		String::Replace(&name, L".fbx", L"");
		String::Replace(&name, L".obj", L"");

		conv->ExportMaterial(L"Weapon/" + name, L"../../_Textures/", false);
		conv->ExportMesh(L"Weapon/" + name, L"../../_Models/");
		SafeDelete(conv);
	}
}

void Convert::Temp()
{

	Converter* conv = new Converter();
	conv->ReadFile(L"Kachujin/S_M_H_Attack.fbx");
	conv->ExportAnimClip(0, L"Kachujin/S_M_H_Attack", L"../../_Models/");
	SafeDelete(conv);
}
