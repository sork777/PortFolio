#include "Framework.h"
#include "Atmosphere.h"
#include "Scattering.h"
#include "Moon.h"
#include "Cloud.h"



Atmosphere::Atmosphere(Shader* shader)
	:Renderer(shader), realTime(false), timeFactor(1), theta(-Math::PI), prevTheta(1)
{
	scatterDesc.InvWaveLength.x = 1.0f / powf(scatterDesc.WaveLength.x, 4.0f);
	scatterDesc.InvWaveLength.y = 1.0f / powf(scatterDesc.WaveLength.y, 4.0f);
	scatterDesc.InvWaveLength.z = 1.0f / powf(scatterDesc.WaveLength.z, 4.0f);

	scatterDesc.WaveLengthMie.x = powf(scatterDesc.WaveLength.x, -0.84f);
	scatterDesc.WaveLengthMie.y = powf(scatterDesc.WaveLength.y, -0.84f);
	scatterDesc.WaveLengthMie.z = powf(scatterDesc.WaveLength.z, -0.84f);

	scattering = new Scattering(shader);
	scatterBuffer = new ConstantBuffer(&scatterDesc, sizeof(ScatterDesc));
	sScatterBuffer = shader->AsConstantBuffer("CB_Scatter");

	cloud = new Cloud(shader);
	cloudBuffer = new ConstantBuffer(&cloudDesc, sizeof(CloudDesc));
	sCloudBuffer = shader->AsConstantBuffer("CB_Cloud");

	moon = new Moon(shader);

	float width = D3D::Width();
	float height = D3D::Height();

	atmoTarget = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	depthStencil = new DepthStencil(width, height);
}


Atmosphere::~Atmosphere()
{
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);

	SafeDelete(moon);
	SafeDelete(cloud);
	SafeDelete(cloudBuffer);
	SafeDelete(scattering);
	SafeDelete(scatterBuffer);
}

void Atmosphere::Update()
{
	Super::Update();
	//Manual
	{
		/* 태양의 움직임에 따라 변경 하기 위함 */
		float x = sinf(theta);
		float y = cosf(theta);

		Context::Get()->LightDirection() = D3DXVECTOR3(x, y, 1.0f);
	}
	////Auto
	//{
	//   theta += Time::Delta() * timeFactor;
	//
	//   if (theta > Math::PI)
	//      theta -= Math::PI;
	//
	//
	//   float x = sinf(theta);
	//   float y = cosf(theta);
	//
	//   Context::Get()->LightDirection() = D3DXVECTOR3(x, y, 0.0f);
	//}
	scattering->Update();
	moon->Update();
	cloud->Update();
}

void Atmosphere::PreRender()
{
	if (theta == prevTheta)
		return;

	prevTheta = theta;


	scatterBuffer->Apply();
	sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

	Super::Render();
	scattering->PreRender();
}

void Atmosphere::Render(bool bRTV)
{
	if(bRTV)
		atmoTarget->Set(depthStencil->DSV());

	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	//Scattering
	{
		position.y += 0.1f;
		
		GetTransform()->Position(position);
		GetTransform()->Scale(1,1,1);
		GetTransform()->Rotation(0, 0, 0);


		scatterDesc.StarIntensity = Context::Get()->LightDirection().y;
		scatterBuffer->Apply();
		sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

		Super::Render();
		scattering->Render();
	}
	//Cloud
	{
		cloudBuffer->Apply();
		sCloudBuffer->SetConstantBuffer(cloudBuffer->Buffer());
 
		GetTransform()->Scale(cloudS);
		GetTransform()->Position(cloudP +position);

		Super::Render();
		cloud->Render();
	}


	//Moon
	{
		scatterDesc.MoonAlpha = moon->MoonAlpha(theta);
		scatterBuffer->Apply();
		sScatterBuffer->SetConstantBuffer(scatterBuffer->Buffer());

		GetTransform()->World(moon->GetTransform(theta));

		Super::Render();
		moon->Render();
	}

	//MoonGlow
	{
		GetTransform()->World(moon->GetGlowTransform(theta));

		Super::Render();
		moon->Render(true);
	}
}

void Atmosphere::PostRender()
{
	scattering->PostRender();
	cloud->PostRender();
}

void Atmosphere::Property()
{
	ImGui::Begin("SkyProperty");
	{
		ImGui::SliderFloat("Theta", &theta, -Math::PI, Math::PI);

		ImGui::BeginChild("CloudProperty");
		{
			cloud->WireCloud();
			ImGui::SliderFloat3("CloudScale", (float*)&cloudS, 0.1f, 20.0f);
			ImGui::SliderFloat3("CloudPos", (float*)&cloudP, -20.0f, 20.0f);
			ImGui::SliderFloat("CloudCover", &cloudDesc.Cover, 0.0f, 1.0f);
			ImGui::SliderFloat("CloudTile", &cloudDesc.Tiles, 0.0f, 2.0f);
			ImGui::EndChild();
		}
		ImGui::End();
	}
}
