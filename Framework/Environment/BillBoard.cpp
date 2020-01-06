#include "Framework.h"
#include "BillBoard.h"

BillBoard::BillBoard(Shader * shader, vector<wstring> & textureNames)
	: Renderer(shader), fixedY(true), bInstancing(false), drawCount(0)
{
	/* ����Ʈ ����Ʈ�ΰ� �ߺ��� */
	//����Ʈ�� �ε����� ����
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	textures = new TextureArray(textureNames, 256, 256);
	shader->AsSRV("Maps")->SetResource(textures->SRV());

	/* �ν��Ͻ� */
	wstring wstr = shader->GetFile();
	string fileName = Path::GetFileName(String::ToString(wstr));
	if (fileName.find("Instance.fx") != string::npos)
	{
		bInstancing = true;
		for (UINT i = 0; i < MAX_INSTANCE; i++)
		{
			transforms[i] = new Transform(shader);
			D3DXMatrixIdentity(&instDesc[i].world);
		}
	
		instanceBuffer = new VertexBuffer(&instDesc, MAX_INSTANCE, sizeof(InstDesc), 1, true);
	}
}

BillBoard::~BillBoard()
{
	SafeDelete(textures);

	//�ν��Ͻ� �ߴٸ� ����
	if (bInstancing == true)
	{
		SafeDelete(instanceBuffer);
		for (UINT i = 0; i < MAX_INSTANCE; i++)
			SafeDelete(transforms[i]);
	}
}

void BillBoard::Add(Vector3 & position, Vector2 & scale,UINT texNum)
{
	VertexScale vertex;
	vertex.Position = position;
	vertex.Scale = scale;
	//�ؽ��� ��ȣ
	//vertex.TexNum = texNum;

	vertices.push_back(vertex);
}

//���� ��ġ ���ϸ� �ٽ� �����
void BillBoard::ResizeBuffer()
{
	//assert(vertexBuffer == NULL);
	if(bInstancing == false && vertices.size()>0)
		vertexBuffer = new VertexBuffer(&vertices[0], vertices.size(), sizeof(VertexScale));
	else if (bInstancing == true)
	{
		/* �⺻���� �� �ϳ�*/
		VertexScale point;
		point.Position = Vector3(0, 0, 0);
		point.Scale = Vector2(1, 1);
		vertexBuffer = new VertexBuffer(&point, 1, sizeof(VertexScale));

		/* �ν��Ͻ̿� */
		/* ������Ʈ���� �ϸ� ������ ����*/
		for (UINT i = 0; i < drawCount; i++)
			memcpy(&instDesc[i].world, &transforms[i]->World(), sizeof(Matrix));

		D3D11_MAPPED_SUBRESOURCE subResource;
		D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		{
			memcpy(subResource.pData, &instDesc, sizeof(InstDesc) * MAX_INSTANCE);
		}
		D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
	}
}

void BillBoard::Clear()
{
	vertices.clear();
	vertices.shrink_to_fit();
}

void BillBoard::Update()
{
	Renderer::Update();

	
}

void BillBoard::Render()
{
	Renderer::Render();

	if (bInstancing == false)
	{
		shader->Draw(0, Pass(), vertices.size());
	}
	else
	{
		instanceBuffer->Render();
		shader->DrawInstanced(0, Pass(), 1, drawCount);
	}
}


/* �ν��Ͻ� */
UINT BillBoard::Push()
{
	drawCount++;
	return drawCount - 1;
}
/*������ ��ġ�� ��������(drawCount-1����)�� �ְ� ũ�⸦ ���δ�.*/
void BillBoard::Pop(UINT index)
{
	instDesc[index] = instDesc[drawCount - 1];
	transforms[index] = transforms[drawCount - 1];
	//�������� ���� �ʱ�ȭ. ���ϸ� �����Ŀ� ������
	transforms[drawCount - 1] = new Transform(shader);
	drawCount--;
}

Transform * BillBoard::GetInstTransform(UINT index)
{
	return transforms[index];
}

UINT BillBoard::GetInstTexNum(UINT index)
{
	return instDesc[index].TexNum; 
}

void BillBoard::SetInstTex(UINT index, UINT TexNum)
{
	instDesc[index].TexNum = TexNum;
}
