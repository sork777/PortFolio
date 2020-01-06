#include "Framework.h"
#include "Vignette.h"

PostEffects::Vignette::Vignette()
	: Render2D(L"033_Vignette.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");

	desc.MapSize = Vector2((float)D3D::GetDesc().Width, (float)D3D::GetDesc().Height);
}

PostEffects::Vignette::~Vignette()
{

}

void PostEffects::Vignette::Update()
{
	Super::Update();

	ImGui::SliderFloat2("Scale", (float *)&desc.Scale, 0.0f, 5.0f);
	ImGui::InputFloat("Vignette", &desc.Vignette, 0.1f);
	//desc.Vignette = Math::Clamp(desc.Vignette, 0.0f, 1.0f);

}

void PostEffects::Vignette::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}