#include "Framework.h"
#include "Saturation.h"

PostEffects::Saturation::Saturation()
	: Render2D(L"033_Saturation.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");
}

PostEffects::Saturation::~Saturation()
{

}

void PostEffects::Saturation::Update()
{
	Super::Update();

	ImGui::PushButtonRepeat(true);
	{
		ImGui::InputFloat("Saturation", &desc.Saturation, 0.1f);
		desc.Saturation = Math::Clamp(desc.Saturation, 0.0f, 1.0f);
	}
	ImGui::PopButtonRepeat();
}

void PostEffects::Saturation::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}