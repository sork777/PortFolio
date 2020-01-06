#include "Framework.h"
#include "Bloom.h"

PostEffects::Bloom::Bloom()
	: Render2D(L"034_Bloom.fx")
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Values");

	desc.MapSize = Vector2(D3D::GetDesc().Width, D3D::GetDesc().Height);

	sLuminosity= shader->AsSRV("LuminosityMap");
	sBlurX= shader->AsSRV("BlurXMap");
	sBlurY= shader->AsSRV("BlurYMap");
	
/*
	FILE* file = NULL;
	fopen_s(&file, "text.csv", "w");
	for (UINT i = 0; i < desc.BloomCount; i++)
	{
		fprintf(
			file,
			"%d,%f,%f,%f,%f,%f,%f\n",
			i,
			desc.BlurX[i].Offset.x, desc.BlurX[i].Offset.y, desc.BlurX[i].Weight,
			desc.BlurY[i].Offset.x, desc.BlurY[i].Offset.y, desc.BlurY[i].Weight
		);
	}
	fclose(file);*/
	
}

PostEffects::Bloom::~Bloom()
{

}

void PostEffects::Bloom::Update()
{
	Super::Update();

	ImGui::InputFloat("Threshold", &desc.Threshold, 0.01f);
	desc.Threshold = Math::Clamp(desc.Threshold,0.0f, 1.0f);
	ImGui::InputFloat("Intensity", &desc.Intensity, 0.01f);

	ImGui::InputInt("BloomCount", (int *)&desc.BloomCount, 2);
	desc.BloomCount %= 33;
	desc.BloomCount = desc.BloomCount < 2 ? 2 : desc.BloomCount;

	desc.BlurX[0].Offset = Vector2(0, 0);
	for (UINT i = 0; i < desc.BloomCount / 2; i++)
	{
		/* uv¿¡ ´õÇÒ °ª*/
		Vector2 offset = Vector2(1.0f / D3D::Width(), 0.0f)*(i*2.0f + 1.5f);
		/* ÁÂ¿ì ´ëÄª? */
		desc.BlurX[i * 2 + 1].Offset = offset;
		desc.BlurX[i * 2 + 2].Offset = -offset;
	}

	desc.BlurY[0].Offset = Vector2(0, 0);
	for (UINT i = 0; i < desc.BloomCount / 2; i++)
	{
		/* uv¿¡ ´õÇÒ °ª*/
		Vector2 offset = Vector2(0.0f, 1.0f / D3D::Height())*(i*2.0f + 1.5f);
		/* ÁÂ¿ì ´ëÄª? */
		desc.BlurY[i * 2 + 1].Offset = offset;
		desc.BlurY[i * 2 + 2].Offset = -offset;
	}

	desc.BlurX[0].Weight = Math::Gaussian(0, desc.BloomCount);
	desc.BlurY[0].Weight = Math::Gaussian(0, desc.BloomCount);
	float weightsX = desc.BlurX[0].Weight;
	float weightsY = desc.BlurY[0].Weight;

	for (UINT i = 0; i < desc.BloomCount / 2; i++)
	{
		float weight = Math::Gaussian((float)(i + 1), desc.BloomCount);
		desc.BlurX[i * 2 + 1].Weight = weight;
		desc.BlurX[i * 2 + 2].Weight = weight;

		desc.BlurY[i * 2 + 1].Weight = weight;
		desc.BlurY[i * 2 + 2].Weight = weight;

		weightsX += weight * 2.0f;
		weightsY += weight * 2.0f;
	}
	for (UINT i = 0; i < desc.BloomCount; i++)
	{
		desc.BlurX[i].Weight /= weightsX;
		desc.BlurY[i].Weight /= weightsY;
	}
}

void PostEffects::Bloom::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	Super::Render();
}

void PostEffects::Bloom::Map(ID3D11ShaderResourceView * l, ID3D11ShaderResourceView * x, ID3D11ShaderResourceView * y)
{
	sLuminosity->SetResource(l);
	sBlurX->SetResource(x);
	sBlurY->SetResource(y);
}
