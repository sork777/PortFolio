#include "Framework.h"
#include "NightVision.h"

PostEffects::NightVision::NightVision()
	: Render2D(L"033_NightVision.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");

	desc.MapSize = Vector2(D3D::Get()->Width(), D3D::Get()->Height());
	texture = new Texture(L"Environment/NightVision2.bmp");
	shader->AsSRV("NightVision")->SetResource(texture->SRV());
}

PostEffects::NightVision::~NightVision()
{

}

void PostEffects::NightVision::Update()
{
	Super::Update();

	ImGui::InputFloat("Weight", &desc.Weight, 0.1f);
	ImGui::InputFloat("Brightness", &desc.Brightness, 0.1f);

}

void PostEffects::NightVision::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}