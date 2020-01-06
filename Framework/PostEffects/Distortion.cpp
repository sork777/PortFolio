#include "Framework.h"
#include "Distortion.h"

PostEffects::Distortion::Distortion()
	: Render2D(L"033_Distortion.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");
}

PostEffects::Distortion::~Distortion()
{

}

void PostEffects::Distortion::Update()
{
	Super::Update();

	ImGui::PushButtonRepeat(true);
	{
		ImGui::InputInt("Pass", (int *)&pass);
		pass %= 3;

		ImGui::InputFloat("Amount", &desc.Noise, 0.01f);
	}
	ImGui::PopButtonRepeat();
}

void PostEffects::Distortion::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}