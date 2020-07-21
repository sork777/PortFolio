#include "Framework.h"
#include "OBBCollider.h"

OBBCollider::OBBCollider()
	:Collider()
{
	Initalize();
}

OBBCollider::~OBBCollider()
{
}

void OBBCollider::Initalize()
{
	lineCount = 12;

	type = CollsionType::OBB;
	
	lines[0] = Vector3(-0.5f, -0.5f, -0.5f);
	lines[1] = Vector3(-0.5f, +0.5f, -0.5f);
	lines[2] = Vector3(+0.5f, -0.5f, -0.5f);
	lines[3] = Vector3(+0.5f, +0.5f, -0.5f);
	lines[4] = Vector3(-0.5f, -0.5f, +0.5f);
	lines[5] = Vector3(-0.5f, +0.5f, +0.5f);
	lines[6] = Vector3(+0.5f, -0.5f, +0.5f);
	lines[7] = Vector3(+0.5f, +0.5f, +0.5f);

	Super::Initalize();
	CreateComputeBuffer();

	rayCB = new ConstantBuffer(&rayDesc, sizeof(RayDesc));
}

void OBBCollider::Update()
{	
	for (int inst = 0; inst < colInfos.size(); inst++)
	{
		//콜라이더 안쓸때만 업뎃 안함
		if (false == colInfos[inst]->bColliderOn) continue;
		
		csInput[inst].data = colInfos[inst]->transform->World();
	}
}

void OBBCollider::Render(const int& draw)
{
	if (false == bDebugMode)
		return;
	int loop = draw > 0 ? draw : colInfos.size();
	for (int inst = 0; inst < loop; inst++)
	{
		//콜라이더 사용 off면 다음것
		if (NULL != frustum && false == csOutput[inst].Frustum) continue;
		if (false == colInfos[inst]->bColliderOn)	continue;

		Vector3 dest[8];
		Matrix world = csInput[inst].data;

		for (UINT i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&dest[i], &lines[i], &world);

		//충돌 여부에 따라 컬러 설정
		Color color = (0 == csOutput[inst].Collision)?norColor:colColor;
		//Color color = norColor;

		RenderLine(dest[0], dest[1], color);
		RenderLine(dest[1], dest[3], color);
		RenderLine(dest[3], dest[2], color);
		RenderLine(dest[2], dest[0], color);


		RenderLine(dest[4], dest[5], color);
		RenderLine(dest[5], dest[7], color);
		RenderLine(dest[7], dest[6], color);
		RenderLine(dest[6], dest[4], color);


		RenderLine(dest[0], dest[4], color);
		RenderLine(dest[1], dest[5], color);
		RenderLine(dest[2], dest[6], color);
		RenderLine(dest[3], dest[7], color);
	}

	Super::Render();	
}

void OBBCollider::Property(const UINT & inst)
{
}


void OBBCollider::RayIntersect(Vector3 & position, Vector3 & direction)
{
	rayDesc.position = position;
	rayDesc.direction = direction;
	rayCB->Apply();

	csShader->AsConstantBuffer("CB_Ray")->SetConstantBuffer(rayCB->Buffer());

	computeBuffer->UpdateInput();
	csShader->AsSRV("Col_InputsA")->SetResource(computeBuffer->SRV());
	csShader->AsUAV("Col_Outputs")->SetUnorderedAccessView(computeBuffer->UAV());
	csShader->Dispatch(1, 0, ceil(GetSize() / 1024.0f), 1, 1);
	// 해당 콜라이더 결과 복사
	computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_COLLISION);
}
const Vector3 & OBBCollider::GetMinRound(const UINT & inst)
{
	if (inst < colInfos.size())
	{
		Vector3 pos, scale;
		colInfos[inst]->transform->Position(&pos);
		colInfos[inst]->transform->Scale(&scale);
		return pos - scale;
	}
	return Vector3();
}
const Vector3 & OBBCollider::GetMaxRound(const UINT & inst)
{
	if (inst < colInfos.size())
	{
		Vector3 pos, scale;
		colInfos[inst]->transform->Position(&pos);
		colInfos[inst]->transform->Scale(&scale);
		return pos + scale;
	}
	return Vector3();
}
const bool & OBBCollider::IsIntersectRay(OUT float& dist, const UINT & inst)
{
	if (inst < colInfos.size())
	{
		dist = csOutput[inst].Dist;
		return csOutput[inst].Collision;
	}
	return false;
}