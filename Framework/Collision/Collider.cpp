#include "Framework.h"
#include "Collider.h"



Collider::Collider()
	
{		
}


Collider::~Collider()
{
	SafeDeleteArray(vertices);
	SafeDelete(vertexBuffer);

	SafeDelete(ctransform);
	SafeDelete(perFrame);
	SafeDelete(shader);
}

void Collider::Initalize()
{
	SelPos = Vector3(-1, -1, -1);
	bDebugMode = false;
	drawCount = 0;

	shader = new Shader(L"027_DebugLine.fx");
	csShader = new Shader(L"Collision.fx");

	UINT vCount = MAX_COLLISION * lineCount;

	vertices = new VertexColor[vCount];
	ZeroMemory(vertices, sizeof(VertexColor) * vCount);

	vertexBuffer = new VertexBuffer(vertices, vCount, sizeof(VertexColor), 0, true);

	perFrame = new PerFrame(shader);
	ctransform = new Transform(shader);
}

void Collider::Render()
{
	// 디버그라인은 기본 랜더방식과 달리 모아서 하기때문에 얘만 따로.
	perFrame->Update();
	ctransform->Update();

	perFrame->Render();
	ctransform->Render(); 
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(VertexColor) * MAX_COLLISION);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);


	vertexBuffer->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	shader->Draw(0, 0, drawCount * 2);

	drawCount = 0;
	ZeroMemory(vertices, sizeof(VertexColor) * MAX_COLLISION*lineCount);
}

void Collider::AddInstance(Transform * transform, Transform * init)
{
	if (MAX_COLLISION >= cmColInfos.size())
		return;	
	CmCol_Info* newCol = new CmCol_Info();
	newCol->bColliderOn = true;
	newCol->transform = transform;
	newCol->init = init;
	
	cmColInfos.emplace_back(&newCol);
}

void Collider::DelInstance(const UINT & inst)
{
	if (inst < cmColInfos.size())
		cmColInfos.erase(cmColInfos.begin() + inst);
}

void Collider::SetColliderOn(const UINT & inst)
{
	if(inst< cmColInfos.size())
		cmColInfos[inst].bColliderOn = true;
}

void Collider::SetColliderOff(const UINT & inst)
{
	if (inst < cmColInfos.size())
		cmColInfos[inst].bColliderOn = false;
}

Transform * Collider::GetTransform(const UINT & inst)
{
	if (inst >= cmColInfos.size()) return NULL;

	return cmColInfos[inst].transform;
}

Transform * Collider::GetInit(const UINT & inst)
{
	if (inst >= cmColInfos.size()) return NULL;
	return cmColInfos[inst].init;
}

void Collider::RenderLine(Vector3 & start, Vector3 & end)
{
	RenderLine(start, end, Color(0, 1, 0, 1));
}

void Collider::RenderLine(Vector3 & start, Vector3 & end, float r, float g, float b)
{
	RenderLine(start, end, Color(r, g, b, 1));
}

void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(0, 1, 0, 1));
}

void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2, float r, float g, float b)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), Color(r, g, b, 1));
}

void Collider::RenderLine(float x, float y, float z, float x2, float y2, float z2, Color & color)
{
	RenderLine(Vector3(x, y, z), Vector3(x2, y2, z2), color);
}

void Collider::RenderLine(Vector3 & start, Vector3 & end, Color & color)
{
	vertices[drawCount].Color = color;
	vertices[drawCount++].Position = start;

	vertices[drawCount].Color = color;
	vertices[drawCount++].Position = end;
}