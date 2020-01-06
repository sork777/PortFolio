#include "Framework.h"
#include "Default.h"

PostEffects::Default::Default()
	: Render2D(L"032_Default.fx")
{
}

PostEffects::Default::~Default()
{

}

void PostEffects::Default::Update()
{
	Super::Update();

	ImGui::InputInt("Pass", (int *)&pass);
	pass %= 6;

	static Color tone = Color(1, 1, 1, 1);
	static Color gamma = Color(1, 1, 1, 1);
	static UINT bits = 0;
	if (pass == 3)
	{
		ImGui::ColorEdit3("Tone", (float*)tone);
		shader->AsVector("Tone")->SetFloatVector(tone);
	}
	else if (pass == 4)
	{
		ImGui::ColorEdit3("Gamma", (float*)gamma);
		shader->AsVector("Gamma")->SetFloatVector(gamma);
	}
	else if (pass == 5)
	{
		ImGui::InputInt("Bits", (int *)&bits);
		bits %= 8;
		shader->AsScalar("Bits")->SetInt(bits);
	}
}

void PostEffects::Default::Render()
{
	Super::Render();
}