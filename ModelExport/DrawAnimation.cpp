#include "stdafx.h"
#include "DrawAnimation.h"
#include "Viewer/Freedom.h"

void DrawAnimation::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(0, 5, -15);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);

	shader = new Shader(L"27_Animation.fx");

	Kachujin();
}

void DrawAnimation::Update()
{
	static UINT clip = 0;

	if (kachujin != NULL)
	{
		if (Keyboard::Get()->Down(VK_SPACE))
		{
			++clip;
			clip %= 4;
			kachujin->PlayClip(20, clip, 1.0f, 1.0f);
		}
			

		kachujin->Update();
	}
}

void DrawAnimation::Render()
{
	if (kachujin != NULL)
	{
		kachujin->Pass(2);
		kachujin->Render();
	}
}

void DrawAnimation::Kachujin()
{
	kachujin = new ModelAnimator(shader);
	kachujin->ReadMaterial(L"Kachujin/Mesh");
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadClip(L"Kachujin/Idle");
	kachujin->ReadClip(L"Kachujin/Running");
	kachujin->ReadClip(L"Kachujin/Jump");
	kachujin->ReadClip(L"Kachujin/Hip_Hop_Dancing");

	weapon = new Model();
	weapon->ReadMaterial(L"Weapon/Sword");
	weapon->ReadMesh(L"Weapon/Sword");


	Transform attachTransform;
	attachTransform.Position(-10, 0, -10);
	attachTransform.Scale(1.0f, 1.0f, 1.0f);

	kachujin->GetModel()->Attach(shader, weapon, 35, &attachTransform);
	kachujin->Pass(2);

	for (float x = -50; x <= 50; x += 2.5f)
	{
		Transform* transform = kachujin->AddTransform();
		transform->Position(x, 0, -5);
		transform->Scale(0.01f, 0.01f, 0.01f);
	}
	kachujin->UpdateTransforms();
}