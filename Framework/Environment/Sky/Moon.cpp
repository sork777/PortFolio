#include "Framework.h"
#include "Moon.h"

Moon::Moon(Shader * shader)
	: shader(shader)
	, distance(415), glowDistance(405)
{
	moon = new Texture(L"Environment/Moon.png");
	moonGlow = new Texture(L"Environment/MoonGlow.png");

	sMoon = shader->AsSRV("MoonMap");


	VertexTexture vertices[6];

	vertices[0].Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices[1].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[2].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[3].Position = Vector3(+1.0f, -1.0f, 0.0f);
	vertices[4].Position = Vector3(-1.0f, +1.0f, 0.0f);
	vertices[5].Position = Vector3(+1.0f, +1.0f, 0.0f);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 1);
	vertices[4].Uv = Vector2(0, 0);
	vertices[5].Uv = Vector2(1, 0);

	vertexBuffer = new VertexBuffer(vertices, 6, sizeof(VertexTexture));
}

Moon::~Moon()
{
	
	SafeDelete(vertexBuffer);
	SafeDelete(moon);
	SafeDelete(moonGlow);
	SafeRelease(sMoon);
}

void Moon::Update()
{
}

void Moon::Render(bool bGlow)
{
	sMoon->SetResource(bGlow ? moonGlow->SRV() : moon->SRV());


	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->Draw(0, 3, 6);
}

float Moon::MoonAlpha(float theta)
{
	if (theta < Math::PI * 0.5f && theta > -Math::PI * 0.5f)
		return fabsf(sinf(theta + Math::PI / 2.0f));

	return 0.0f;
}

D3DXMATRIX Moon::GetTransform(float theta)
{
	Vector3 position;
	Context::Get()->GetCamera()->Position(&position);

	Matrix S, R, T, D;
	D3DXMatrixScaling(&S, 50, 50, 1);
	D3DXMatrixRotationYawPitchRoll(&R, Math::PI * 0.5f, theta - (Math::PI * 0.5f), 0);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	Vector3 direction = Context::Get()->LightDirection();
	D3DXMatrixTranslation
	(
		&D
		, direction.x * distance
		, direction.y * distance
		, direction.z * distance
	);

	return S * R * T * D;
}

D3DXMATRIX Moon::GetGlowTransform(float theta)
{
	D3DXVECTOR3 position;
	Context::Get()->GetCamera()->Position(&position);

	D3DXMATRIX S, R, T, D;
	D3DXMatrixScaling(&S, 150, 150, 1);
	D3DXMatrixRotationYawPitchRoll(&R, Math::PI * 0.5f, theta - (Math::PI * 0.5f), 0);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	D3DXVECTOR3 direction = Context::Get()->LightDirection();
	D3DXMatrixTranslation
	(
		&D
		, direction.x * glowDistance
		, direction.y * glowDistance
		, direction.z * glowDistance
	);

	return S * R * T * D;
}