#include "Framework.h"
#include "ColliderTest.h"

ColliderTest::ColliderTest()	
{		
	shader = SETSHADER(L"027_DebugLine.fx");
	csShader = SETSHADER(L"ColliderTest/Collision.fx");
}

ColliderTest::ColliderTest(Shader * shader, Shader * cs)
	:shader(shader),csShader(cs)
{
}


ColliderTest::~ColliderTest()
{
	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);

	SafeDelete(ctransform);
	SafeDelete(perFrame);
	
}

void ColliderTest::Initalize()
{
	bDebugMode = false;
	drawCount = 0;


	vCount = MAX_COLLISION * lineCount*2;

	vertices = new VertexColor[vCount];
	ZeroMemory(vertices, sizeof(VertexColor) * vCount);

	vertexBuffer = new VertexBuffer(vertices, vCount, sizeof(VertexColor), 0, true);

	perFrame = new PerFrame(shader);
	ctransform = new Transform(shader);
}

void ColliderTest::Render(const int& draw)
{
	// ����׶����� �⺻ ������İ� �޸� ��Ƽ� �ϱ⶧���� �길 ����.
	perFrame->Update();
	ctransform->Update();

	perFrame->Render();
	ctransform->Render(); 
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(VertexColor) * vCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);


	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	shader->Draw(0, 0, drawCount * 2);

	drawCount = 0;
	ZeroMemory(vertices, sizeof(VertexColor) * vCount);
}

#pragma region �ν��Ͻ� ���� ����

void ColliderTest::AddInstance(Transform * transform , Transform * init )
{
	if (MAX_COLLISION <= colInfos.size())
		return;
	Col_Info* newCol = new Col_Info();
	newCol->transform = transform;
	newCol->init = init;

	colInfos.emplace_back(newCol);
}

void ColliderTest::DelInstance(const UINT & inst)
{
	if (inst < colInfos.size())
		colInfos.erase(colInfos.begin() + inst);
}

Transform * ColliderTest::GetTransform(const UINT & inst)
{
	if (inst < colInfos.size())
		return colInfos[inst]->transform;
	return nullptr;
}

Transform * ColliderTest::GetInit(const UINT & inst)
{
	if (inst < colInfos.size())
		return colInfos[inst]->init;
	return nullptr;
}

void ColliderTest::SetColliderTestOn(const UINT & inst)
{
	if (inst < colInfos.size())
		colInfos[inst]->bColliderOn = true;
}

void ColliderTest::SetColliderTestOff(const UINT & inst)
{
	if (inst < colInfos.size())
		colInfos[inst]->bColliderOn = false;
}

const bool & ColliderTest::IsCollisionOn(const UINT & inst)
{
	if (inst < colInfos.size())
		return colInfos[inst]->bColliderOn;

	return false;
}
const bool & ColliderTest::IsCollision(const UINT & inst)
{
	if (inst < colInfos.size())
		return csOutput[inst].Collision;
	return false;
}
#pragma endregion

#pragma region CS ����

void ColliderTest::CSColliderTestB()
{
	//input������ ����
	computeBuffer->UpdateInput();
	csShader->AsSRV("Col_InputsB")->SetResource(computeBuffer->SRV());
}

void ColliderTest::ComputeColliderTest(const UINT& tech, const UINT& pass, ColliderTest * colB)
{
	// �ش� �ݶ��̴��� ��ǲ ������Ʈ �� ���������� �Է�
	computeBuffer->UpdateInput();
	csShader->AsSRV("Col_InputsA")->SetResource(computeBuffer->SRV());
	// �ƿ�ǲ�� 2������ �޸� �����Ǽ� ������
	csShader->AsUAV("Col_Outputs")->SetUnorderedAccessView(computeBuffer->UAV());
	//�ڱ� �ڽŰ��� �浹
	if (colB == NULL || this == colB)
	{
		csShader->Dispatch(tech, pass, ceil(GetSize()/1024.0f), GetSize(), 1);
	}
	//colB���� �浹
	else
	{
		//colB�� B�������� ��ǲ ���� �� ���������� �Է�
		colB->CSColliderTestB();
		csShader->Dispatch(tech, pass, ceil(GetSize() / 1024.0f), colB->GetSize(), 1);
	}
	// �ش� �ݶ��̴� ��� ����
	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_COLLISION);
}

void ColliderTest::CreateComputeBuffer()
{
	UINT outSize = MAX_COLLISION;
	csInput = new CS_InputDesc[outSize];
	csOutput = new CS_OutputDesc[outSize];

	computeBuffer = new StructuredBuffer
	(
		csInput,
		sizeof(CS_InputDesc), outSize,
		true,
		sizeof(CS_OutputDesc), outSize
	);
}
#pragma endregion


#pragma region DebugLine ����

void ColliderTest::RenderLine(Vector3 & start, Vector3 & end)
{
	RenderLine(start, end, Color(0, 1, 0, 1));
}

void ColliderTest::RenderLine(Vector3 & start, Vector3 & end, float r, float g, float b)
{
	RenderLine(start, end, Color(r, g, b, 1));
}

void ColliderTest::RenderLine(float x, float y, float z, float x2, float y2, float z2)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(0, 1, 0, 1));
}

void ColliderTest::RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(r, g, b, 1));
}

void ColliderTest::RenderLine(float x, float y, float z, float x2, float y2, float z2, Color & color)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), color);
}

void ColliderTest::RenderLine(Vector3 & start, Vector3 & end, Color & color)
{
	vertices[drawCount].Color = color;
	vertices[drawCount++].Position = start;

	vertices[drawCount].Color = color;
	vertices[drawCount++].Position = end;
}

#pragma endregion
