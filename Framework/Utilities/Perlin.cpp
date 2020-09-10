#include "Framework.h"
#include "Perlin.h"
#include <random>
#include <algorithm>
#include <numeric>

Perlin::Perlin(UINT width , UINT height,DXGI_FORMAT format)
	:format(format)
{
	UINT w = width;
	UINT h = height;
	if (width < 1)
		w = 512;
	if (height < 1)
		h = 512;

	shader = SETSHADER(L"HW00_Perlin.fx");
	cbuffer = new ConstantBuffer(&perlinDesc, sizeof(PerlinDesc));
	NoiseCs = new CsTexture(w,h,format);

	perlinDesc.BottomColor = Color(0, 0, 0, 0);
	perlinDesc.TopColor = Color(1, 1, 1, 1);
	perlinDesc.Octave = 5;
	perlinDesc.Persistence = 0.4f;
	perlinDesc.SmoothedN = 0;

	perlinDesc.Res = Vector2(w,h);
	perlinDesc.GridSize = 2.5f;
	perlinDesc.Seed = 0;
}


Perlin::~Perlin()
{
	
	SafeDelete(cbuffer);
	SafeDelete(NoiseCs);
}

void Perlin::PerlinController()
{
	bool bDocking = true;
	bool bChange = false;
	bool bSeedChange = false;
	ImGui::Begin("PerlinGenerator", &bDocking);
	{
		bChange |= ImGui::ColorEdit4("BottomColor", (float*)&perlinDesc.BottomColor);
		bChange |= ImGui::ColorEdit4("TopColor", (float*)&perlinDesc.TopColor);
		ImGui::Image(NoiseCs->SRV(), ImVec2(120, 120));
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			ImGui::PushItemWidth(100);
			bChange |= (bSeedChange = ImGui::SliderInt("Seed", &perlinDesc.Seed, 1, 100));
			bChange |=ImGui::SliderInt("Octave", &perlinDesc.Octave, 3, 10);
			bChange |=ImGui::SliderFloat("Persistence", &perlinDesc.Persistence, 0.1f, 1.0f);
			bChange |=ImGui::SliderFloat("GridSize", &perlinDesc.GridSize, 0.1f, 10.0f);
			ImGui::PopItemWidth();
		}
		ImGui::EndGroup();
		bChange |=ImGui::RadioButton("Smoothed1", &perlinDesc.SmoothedN, 0);
		bChange |=ImGui::RadioButton("Smoothed2", &perlinDesc.SmoothedN, 1);
		//변화 하나라도 있으면 사용 중지
		bPerlinUse = bChange ? false : bPerlinUse;
		if (ImGui::Button("CreatePixels"))
		{
			ID3D11Texture2D* srcTexture;
			NoiseCs->SRV()->GetResource((ID3D11Resource **)&srcTexture);
			Texture::ReadPixels(srcTexture, DXGI_FORMAT(format+1), &PerlinPixel);
			bPerlinUse = true;
		}
	}
	ImGui::End();
}

void Perlin::GeneratorNoise2D()
{
	cbuffer->Apply();
	shader->AsConstantBuffer("CB_Perlin")->SetConstantBuffer(cbuffer->Buffer());
	shader->AsUAV("PerlinTex")->SetUnorderedAccessView(NoiseCs->UAV());
	int size = perlinDesc.Res.x*perlinDesc.Res.y;
	shader->Dispatch(0, 0, size / 1024, 1, 1);
}

void Perlin::Resize(UINT size)
{
	int s=1;
	//2의 지승이 되어야 함...
	while (s < size*2)
		s *= 2;

	if (size < 1)
		s = 512;
	else if (size < 16)		//최소 사이즈
		s = 16;

	NoiseCs->Resize(s, s);
	perlinDesc.Res = Vector2(s,s);
	//ReSeed();
}
//
//void Perlin::ReSeed()
//{
//	p.resize(perlinDesc.Res.x*0.5f);
//
//	// Fill p with values from 0 to Size
//	std::iota(p.begin(), p.end(), 0);
//
//	// Initialize a random engine with seed
//	std::default_random_engine engine(perlinDesc.Seed);
//
//	// Suffle  using the above random engine
//	std::shuffle(p.begin(), p.end(), engine);
//
//	// Duplicate the permutation vector
//	p.insert(p.end(), p.begin(), p.end());
//
//	shader->AsScalar("Permute")->SetIntArray((int*)&p, 0, p.size());
//}
