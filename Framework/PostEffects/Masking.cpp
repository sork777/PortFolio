#include "Framework.h"
#include "Masking.h"

PostEffects::Masking::Masking()
	: Render2D(L"033_Mask.fx")
{
}

PostEffects::Masking::~Masking()
{

}

void PostEffects::Masking::Update()
{
	Super::Update();

	ImGui::InputInt("Pass", (int *)&pass);
	pass %= 3;

	Vector3 size;
	this->GetTransform()->Scale(&size);
	shader->AsVector("Size")->SetFloatVector(size);
}

void PostEffects::Masking::Render()
{
	Super::Render();
}