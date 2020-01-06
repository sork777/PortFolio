#include "Framework.h"
#include "Wiggle.h"

PostEffects::Wiggle::Wiggle()
	: Render2D(L"033_Wiggle.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");
}

PostEffects::Wiggle::~Wiggle()
{

}

void PostEffects::Wiggle::Update()
{
	Super::Update();

	ImGui::SliderFloat2("Offset", (float*)desc.Offset,0.0f,100.0f);
	ImGui::SliderFloat2("Amount", (float*)desc.Amount,0.01f,1.0f);

}

void PostEffects::Wiggle::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}