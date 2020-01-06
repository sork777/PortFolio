#include "Framework.h"
#include "Sepia.h"

PostEffects::Sepia::Sepia()
	: Render2D(L"033_Sepia.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");

	/* Åæ °ª */
	desc.Sepia = Matrix
	(
		0.393f, 0.769f, 0.189f, 0,
		0.349f, 0.686f, 0.168f, 0,
		0.272f, 0.534f, 0.131f, 0,
		0, 0, 0, 1
	);
	D3DXMatrixTranspose(&desc.Sepia, &desc.Sepia);
}

PostEffects::Sepia::~Sepia()
{

}

void PostEffects::Sepia::Update()
{
	Super::Update();

	ImGui::PushButtonRepeat(true);
	{
		ImGui::InputInt("Pass", (int *)&pass);
		pass %= 2;

		ImGui::InputFloat("Strength", &desc.Strength, 0.1f);
		desc.Strength=Math::Clamp(desc.Strength, 0.0f, 1.0f);
	}
	ImGui::PopButtonRepeat();
}

void PostEffects::Sepia::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}