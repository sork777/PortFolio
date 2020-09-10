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

	// �ؽ�ó ��ǥ ���� ���� ����մϴ�.
	float textureDelta = (float)2 / (float)planeRes;
	UINT index = 0;
	VertexTexture* vertices = new VertexTexture[domeVertexCount];

	// �ϴ� ����� �ݺ��ϰ� ������ ���� ���� ������� ��ǥ�� ����ϴ�.
	for (UINT i = 0; i <= planeRes; i++)
	{
		for (UINT j = 0; j <= planeRes; j++)
		{
			// ���� ��ǥ�� ����մϴ�.
			//-0.5f * domeCount~+0.5f * domeCount, (-0.5f * domeCount)+(i*domeCount/ planeRes)
			positionX = (-0.5f * domeCount) + ((float)i * quadSize);
			positionZ = (-0.5f * domeCount) + ((float)j * quadSize);
			//positionY = ((positionX * positionX) + (positionZ * positionZ));
			float tempX = positionX / domeCount;	//(0.5f+i/Res)
			float tempY = positionY / domeCount;	//(0.5f+j/Res)
			
			//Top-(Top-Bottom)*4*((0.5f+i/Res)^2+(0.5f+j/Res)^2)
			positionY = skyTop -(constant * ((positionX * positionX) + (positionZ * positionZ)))*4.0f;
			
			if (positionY < skyBottom+0.0f)
				positionY = skyBottom+0.0f;

			// �ؽ�ó ��ǥ�� ����մϴ�.
			tu = (float)i * textureDelta;
			tv = (float)j * textureDelta;

			// �� ��ǥ�� �߰��ϱ� ���� �ϴ� ��� �迭�� �ε����� ����մϴ�.
			index = i * (planeRes + 1) + j;

			// �ϴ� ��� �迭�� ��ǥ�� �߰��մϴ�.
			vertices[index].Position.x = positionX;
			vertices[index].Position.y = positionY-skyBottom;
			vertices[index].Position.z = positionZ;
			D3DXVec3Normalize(&vertices[index].Position, &vertices[index].Position);
			vertices[index].Uv.x = tu;
			vertices[index].Uv.y = tv;
			//D3DXVec2Normalize(&vertices[index].Uv, &vertices[index].Uv);
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
