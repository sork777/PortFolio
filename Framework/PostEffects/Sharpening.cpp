#include "Framework.h"
#include "Sharpening.h"

PostEffects::Sharpening::Sharpening()
	: Render2D(L"033_Sharpening.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");

	desc.MapSize = Vector2(D3D::GetDesc().Width, D3D::GetDesc().Height);
}

PostEffects::Sharpening::~Sharpening()
{

}

void PostEffects::Sharpening::Update()
{
	Super::Update();

	ImGui::PushButtonRepeat(true);
	{
		ImGui::InputInt("Pass", (int *)&pass);
		pass %= 2;
		ImGui::InputFloat("Sharpening", &desc.Sharpening, 0.1f);
		desc.Sharpening = Math::Clamp(desc.Sharpening, 0.0f, 1.0f);
	}
	ImGui::PopButtonRepeat();
}

void PostEffects::Sharpening::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}