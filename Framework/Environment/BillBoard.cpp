#include "Framework.h"
#include "BillBoard.h"

BillBoard::BillBoard(Shader * shader, vector<wstring> & textureNames)
	: Renderer(shader), fixedY(true), bInstancing(false), drawCount(0)
{
	/* 포인트 리스트인것 잘보기 */
	//포인트라 인덱스도 없음
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	textures = new TextureArray(textureNames, 256, 256);
	shader->AsSRV("Maps")->SetResource(textures->SRV());

	/* 인스턴싱 */
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

	//인스턴싱 했다면 삭제
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
	//텍스쳐 번호
	//vertex.TexNum = texNum;

	vertices.push_back(vertex);
}

//정점 위치 변하면 다시 해줘야
void BillBoard::ResizeBuffer()
{
	//assert(vertexBuffer == NULL);
	if(bInstancing == false && vertices.size()>0)
		vertexBuffer = new VertexBuffer(&vertices[0], vertices.size(), sizeof(VertexScale));
	else if (bInstancing == true)
	{
		/* 기본적인 점 하나*/
		VertexScale point;
		point.Position = Vector3(0, 0, 0);
		point.Scale = Vector2(1, 1);
		vertexBuffer = new VertexBuffer(&point, 1, sizeof(VertexScale));

		/* 인스턴싱용 */
		/* 업데이트에서 하면 오히려 느림*/
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


/* 인스턴싱 */
UINT BillBoard::Push()
{
	drawCount++;
	return drawCount - 1;
}
/*선택한 위치에 마지막값(drawCount-1번쨰)을 넣고 크기를 줄인다.*/
void BillBoard::Pop(UINT index)
{
	instDesc[index] = instDesc[drawCount - 1];
	transforms[index] = transforms[drawCount - 1];
	//마지막은 새로 초기화. 안하면 삭제후에 에러남
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
