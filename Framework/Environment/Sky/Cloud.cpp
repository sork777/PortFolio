#include "Framework.h"
#include "Cloud.h"
#include <fstream>
using namespace std;

Cloud::Cloud(Shader * shader)
	: shader(shader)
{
	sSRV = shader->AsSRV("CloudMap");

	render2D = new Render2D();
	render2D->GetTransform()->Position(400 + 100, 0 + 100, 0);
	render2D->GetTransform()->Scale(200, 200, 1);

	cloud = new Texture(L"Environment/cloud002.dds");
	sSRV->SetResource(cloud->SRV());
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
	/*

	int perm[] =
	{
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int gradValues[] =
	{
		+1, +1, +0, -1, +1, +0, +1, -1, +0, -1, -1, +0,
		+1, +0, +1, -1, +0, +1, +1, +0, -1, -1, +0, -1,
		+0, +1, +1, +0, -1, +1, +0, +1, -1, +0, -1, -1,
		+1, +1, +0, +0, -1, +1, -1, +1, +0, +0, -1, -1
	};


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = 256;
	desc.Height = 256;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	Color* pixels = new Color[256 * 256];
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			int value = perm[(x + perm[y]) & 0xFF];

			Color color;
			color.r = (float)(gradValues[value & 0x0F] * 64 + 64);
			color.g = (float)(gradValues[value & 0x0F + 1] * 64 + 64);
			color.b = (float)(gradValues[value & 0x0F + 2] * 64 + 64);
			color.a = (float)value;


			UINT index = desc.Width * y + x;
			pixels[index] = color;
		}
	}

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = 256 * 4;

	Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &texture));

	D3DX11SaveTextureToFile(D3D::GetDC(), texture, D3DX11_IFF_PNG, L"Noise.png");


	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	SafeDeleteArray(pixels);
	*/

	//CreateDome();
	CreatePlane();
}

Cloud::~Cloud()
{

	SafeDelete(vertexBuffer);
	SafeDelete(domeVertexBuffer);
	SafeDelete(domeIndexBuffer);
}

void Cloud::Update()
{
	render2D->Update();
}

void Cloud::Render(bool bGlow)
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;
	//vertexBuffer->Render();
	domeVertexBuffer->Render();
	domeIndexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//shader->Draw(0, 2, 6);
	static bool bwire=false;
	ImGui::Checkbox("WireCloud", &bwire);
	if(bwire)
		shader->DrawIndexed(0, 5, domeIndexCount);
	else
		shader->DrawIndexed(0, 4, domeIndexCount);
}

void Cloud::PostRender()
{
	render2D->SRV(srv);
	render2D->Render();
}

void Cloud::CreateDome()
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
			vertices[index].Position.y = cosf(xz);
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
			vertices[index].Position.y = cosf(xz);
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

void Cloud::CreatePlane()
{
	
	float quadSize = (float)domeCount/ (float)planeRes; // 
	float radius = (float)domeCount / 2.0f; // 
	//(Top-Bottom)*4/(domeCount^2)
	float constant = (skyTop - skyBottom) / (radius * radius);

	domeVertexCount = (planeRes+1) * (planeRes + 1);
	domeIndexCount = domeVertexCount*6;
	float positionX = 0.0f;
	float positionY = 0.0f;
	float positionZ = 0.0f;
	float tu = 0.0f;
	float tv = 0.0f;

	// 텍스처 좌표 증가 값을 계산합니다.
	float textureDelta = (float)2 / (float)planeRes;
	UINT index = 0;
	VertexTexture* vertices = new VertexTexture[domeVertexCount];

	// 하늘 평면을 반복하고 제공된 증분 값을 기반으로 좌표를 만듭니다.
	for (UINT i = 0; i <= planeRes; i++)
	{
		for (UINT j = 0; j <= planeRes; j++)
		{
			// 정점 좌표를 계산합니다.
			//-0.5f * domeCount~+0.5f * domeCount, (-0.5f * domeCount)+(i*domeCount/ planeRes)
			positionX = (-0.5f * domeCount) + ((float)i * quadSize);
			positionZ = (-0.5f * domeCount) + ((float)j * quadSize);
			//positionY = ((positionX * positionX) + (positionZ * positionZ));
			float tempX = positionX / domeCount;	//(0.5f+i/Res)
			float tempY = positionY / domeCount;	//(0.5f+j/Res)
			//Top-(Top-Bottom)*4*((0.5f+i/Res)^2+(0.5f+j/Res)^2)
			positionY = skyTop-(constant * ((positionX * positionX) + (positionZ * positionZ)));
			
			// 텍스처 좌표를 계산합니다.
			tu = (float)i * textureDelta;
			tv = (float)j * textureDelta;

			// 이 좌표를 추가하기 위해 하늘 평면 배열에 인덱스를 계산합니다.
			index = i * (planeRes + 1) + j;

			// 하늘 평면 배열에 좌표를 추가합니다.
			vertices[index].Position.x = positionX;
			vertices[index].Position.y = positionY;
			vertices[index].Position.z = positionZ;
			vertices[index].Uv.x = tu;
			vertices[index].Uv.y = tv;
		}
	}

	index = 0;
	UINT* indices = new UINT[domeIndexCount];
	
	for (int i = 0; i < planeRes; i++)
	{
		for (int j = 0; j < planeRes; j++)
		{
			indices[index++] = i * (planeRes+1) + j;
			indices[index++] = (i + 1) * (planeRes+1) + j;
			indices[index++] = (i + 1) * (planeRes+1) + (j + 1);

			indices[index++] = (i + 1) * (planeRes+1) + (j + 1);
			indices[index++] = i * (planeRes + 1) + (j + 1);
			indices[index++] = i * (planeRes + 1) + j ;
			
		}
	}
	domeVertexBuffer = new VertexBuffer(vertices, domeVertexCount, sizeof(VertexTexture), 0);
	domeIndexBuffer = new IndexBuffer(indices, domeIndexCount);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}
