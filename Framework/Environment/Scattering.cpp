#include "Framework.h"
#include "Scattering.h"

Scattering::Scattering(Shader * shader)
	: shader(shader), width(256), height(128), domeCount(32)
{
	mieTarget = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	rayleighTarget = new RenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	depthStencil = new DepthStencil(width, height);
	viewport = new Viewport((float)width, (float)height);

	render2D = new Render2D();
	render2D->GetTransform()->Scale(200, 200, 1);
	render2D->GetTransform()->RotationDegree(0, 0, -90);
	CreateQuad();
	CreateDome();

	sRayleighSRV = shader->AsSRV("RayleighMap");
	sMieSRV = shader->AsSRV("MieMap");
}


Scattering::~Scattering()
{
	SafeDelete(render2D);

	SafeDelete(mieTarget);
	SafeDelete(rayleighTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);

	SafeDelete(vertexBuffer);
	SafeDelete(domeVertexBuffer);
	SafeDelete(domeIndexBuffer);
}

void Scattering::Update()
{
	render2D->Update();
}

void Scattering::PreRender()
{
	vector<RenderTarget *> rtvs;
	rtvs.push_back(rayleighTarget);
	rtvs.push_back(mieTarget);
	RenderTarget::Sets(rtvs, depthStencil->DSV());

	viewport->RSSetViewport();


	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->Draw(0, 0, 6);
}

void Scattering::Render()
{

	sRayleighSRV->SetResource(rayleighTarget->SRV());
	sMieSRV->SetResource(mieTarget->SRV());
	//sStar->SetResource(texture->SRV());


	domeVertexBuffer->Render();
	domeIndexBuffer->Render();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, 1, domeIndexCount);
}

void Scattering::PostRender()
{
	render2D->SRV(rayleighTarget->SRV());
	render2D->GetTransform()->Position(0 + 100, 0 + 100, 0);
	render2D->Update();
	render2D->Render();

	render2D->SRV(mieTarget->SRV());
	render2D->GetTransform()->Position(200 + 100, 0 + 100, 0);
	render2D->Update();
	render2D->Render();
}

void Scattering::CreateQuad()
{
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

void Scattering::CreateDome()
{
	UINT latitude = domeCount / 2; // 위도
	UINT longitude = domeCount; // 경도

	domeVertexCount = longitude * latitude * 2;
	domeIndexCount = (longitude - 1) * (latitude - 1) * 2 * 8;


	VertexTexture* vertices = new VertexTexture[domeVertexCount];

	UINT index = 0;
	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz)*2;
			vertices[index].Position.z = sinf(xz) * sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)

	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI * j / (latitude - 1));

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz)*2;
			vertices[index].Position.z = sinf(xz) * sinf(y);

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)


	index = 0;
	UINT* indices = new UINT[domeIndexCount * 3];

	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + (j + 1);

			indices[index++] = (i + 1) * latitude + (j + 1);
			indices[index++] = i * latitude + (j + 1);
			indices[index++] = i * latitude + j;
		}
	}

	UINT offset = latitude * longitude;
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + i * latitude + j;
		}
	}

	domeVertexBuffer = new VertexBuffer(vertices, domeVertexCount, sizeof(VertexTexture), 0);
	domeIndexBuffer = new IndexBuffer(indices, domeIndexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}
