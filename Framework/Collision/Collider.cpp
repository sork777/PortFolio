#include "Framework.h"
#include "Collider.h"




Collider::Collider(Transform * transform, Transform * init)
	:transform(transform), init(init), SelPos(-1, -1, -1)
	, bUsingCollider(true)
{

}


Collider::~Collider()
{
}
//
//Collider::Collider()	
//{		
//	shader = SETSHADER(L"027_DebugLine.fx");
//	csShader = SETSHADER(L"Collider/Collision.fx");
//}
//
//Collider::Collider(Shader * shader, Shader * cs)
//	:shader(shader),csShader(cs)
//{
//}
//
//
//Collider::~Collider()
//{
//	SafeDeleteArray(vertices);
//	SafeDelete(vertexBuffer);
//
//	SafeDelete(ctransform);
//	SafeDelete(perFrame);
//	
//}
//
//void Collider::Initalize()
//{
//	bDebugMode = false;
//	drawCount = 0;
//
//
//	vCount = MAX_COLLISION * lineCount*2;
//
//	vertices = new VertexColor[vCount];
//	ZeroMemory(vertices, sizeof(VertexColor) * vCount);
//
//	vertexBuffer = new VertexBuffer(vertices, vCount, sizeof(VertexColor), 0, true);
//
//	perFrame = new PerFrame(shader);
//	ctransform = new Transform(shader);
//}
//
//void Collider::Render(const int& draw)
//{
//	// ����׶����� �⺻ ������İ� �޸� ��Ƽ� �ϱ⶧���� �길 ����.
//	perFrame->Update();
//	ctransform->Update();
//
//	perFrame->Render();
//	ctransform->Render(); 
//	D3D11_MAPPED_SUBRESOURCE subResource;
//	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
//	{
//		memcpy(subResource.pData, vertices, sizeof(VertexColor) * vCount);
//	}
//	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
//
//
//	vertexBuffer->Render();
//	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//	shader->Draw(0, 0, drawCount * 2);
//
//	drawCount = 0;
//	ZeroMemory(vertices, sizeof(VertexColor) * vCount);
//}
//
//#pragma region �ν��Ͻ� ���� ����
//
//void Collider::AddInstance(Transform * transform , Transform * init )
//{
//	if (MAX_COLLISION <= colInfos.size())
//		return;
//	Col_Info* newCol = new Col_Info();
//	newCol->transform = transform;
//	newCol->init = init;
//
//	colInfos.emplace_back(newCol);
//}
//
//void Collider::DelInstance(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		colInfos.erase(colInfos.begin() + inst);
//}
//
//Transform * Collider::GetTransform(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return colInfos[inst]->transform;
//	return nullptr;
//}
//
//Transform * Collider::GetInit(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return colInfos[inst]->init;
//	return nullptr;
//}
//
//void Collider::SetColliderOn(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		colInfos[inst]->bColliderOn = true;
//}
//
//void Collider::SetColliderOff(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		colInfos[inst]->bColliderOn = false;
//}
//
//const bool & Collider::IsCollisionOn(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return colInfos[inst]->bColliderOn;
//
//	return false;
//}
//const bool & Collider::IsCollision(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return csOutput[inst].Collision;
//	return false;
//}
//#pragma endregion
//
//#pragma region CS ����
//
//void Collider::CSColliderB()
//{
//	//input������ ����
//	computeBuffer->UpdateInput();
//	csShader->AsSRV("Col_InputsB")->SetResource(computeBuffer->SRV());
//}
//
//void Collider::ComputeCollider(const UINT& tech, const UINT& pass, Collider * colB)
//{
//	// �ش� �ݶ��̴��� ��ǲ ������Ʈ �� ���������� �Է�
//	computeBuffer->UpdateInput();
//	csShader->AsSRV("Col_InputsA")->SetResource(computeBuffer->SRV());
//	// �ƿ�ǲ�� 2������ �޸� �����Ǽ� ������
//	csShader->AsUAV("Col_Outputs")->SetUnorderedAccessView(computeBuffer->UAV());
//	//�ڱ� �ڽŰ��� �浹
//	if (colB == NULL || this == colB)
//	{
//		csShader->Dispatch(tech, pass, ceil(GetSize()/1024.0f), GetSize(), 1);
//	}
//	//colB���� �浹
//	else
//	{
//		//colB�� B�������� ��ǲ ���� �� ���������� �Է�
//		colB->CSColliderB();
//		csShader->Dispatch(tech, pass, ceil(GetSize() / 1024.0f), colB->GetSize(), 1);
//	}
//	// �ش� �ݶ��̴� ��� ����
//	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_COLLISION);
//}
//
//void Collider::CreateComputeBuffer()
//{
//	UINT outSize = MAX_COLLISION;
//	csInput = new CS_InputDesc[outSize];
//	csOutput = new CS_OutputDesc[outSize];
//
//	computeBuffer = new StructuredBuffer
//	(
//		csInput,
//		sizeof(CS_InputDesc), outSize,
//		true,
//		sizeof(CS_OutputDesc), outSize
//	);
//}
//#pragma endregion
//
//
//#pragma region DebugLine ����
//
//void Collider::RenderLine(Vector3 & start, Vector3 & end)
//{
//	RenderLine(start, end, Color(0, 1, 0, 1));
//}
//
//void Collider::RenderLine(Vector3 & start, Vector3 & end, float r, float g, float b)
//{
//	RenderLine(start, end, Color(r, g, b, 1));
//}
//
//void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2)
//{
//	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(0, 1, 0, 1));
//}
//
//void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b)
//{
//	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(r, g, b, 1));
//}
//
//void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2, Color & color)
//{
//	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), color);
//}
//
//void Collider::RenderLine(Vector3 & start, Vector3 & end, Color & color)
//{
//	vertices[drawCount].Color = color;
//	vertices[drawCount++].Position = start;
//
//	vertices[drawCount].Color = color;
//	vertices[drawCount++].Position = end;
//}
//
//#pragma endregion
