#include "Framework.h"
#include "OBBCollider.h"

OBBCollider::OBBCollider(Transform * transform, Transform * init)
	:Collider(transform, init)
{
	type = CollsionType::OBB;
	if (transform == NULL)
		this->transform = new Transform();

	lines[0] = Vector3(-0.5f, -0.5f, -0.5f);
	lines[1] = Vector3(-0.5f, +0.5f, -0.5f);
	lines[2] = Vector3(+0.5f, -0.5f, -0.5f);
	lines[3] = Vector3(+0.5f, +0.5f, -0.5f);
	lines[4] = Vector3(-0.5f, -0.5f, +0.5f);
	lines[5] = Vector3(-0.5f, +0.5f, +0.5f);
	lines[6] = Vector3(+0.5f, -0.5f, +0.5f);
	lines[7] = Vector3(+0.5f, +0.5f, +0.5f);

	Update();
}

OBBCollider::~OBBCollider()
{
}

void OBBCollider::Update()
{
	transform->Update();
	if (init != NULL)
		init->Update();
	if (bUsingCollider == false)
		return;
	SetObb();
}

void OBBCollider::Render(Color color)
{
	if (bUsingCollider == false)
		return;

	Vector3 dest[8];
	Matrix world = transform->World();
	transform->Render();
	if (init != NULL)
	{
		init->Render();
		world = init->World()*transform->World();
	}

	for (UINT i = 0; i < 8; i++)
		D3DXVec3TransformCoord(&dest[i], &lines[i], &world);


	//Front
	DebugLine::Get()->RenderLine(dest[0], dest[1], color);
	DebugLine::Get()->RenderLine(dest[1], dest[3], color);
	DebugLine::Get()->RenderLine(dest[3], dest[2], color);
	DebugLine::Get()->RenderLine(dest[2], dest[0], color);

	//Backward
	DebugLine::Get()->RenderLine(dest[4], dest[5], color);
	DebugLine::Get()->RenderLine(dest[5], dest[7], color);
	DebugLine::Get()->RenderLine(dest[7], dest[6], color);
	DebugLine::Get()->RenderLine(dest[6], dest[4], color);

	//Side
	DebugLine::Get()->RenderLine(dest[0], dest[4], color);
	DebugLine::Get()->RenderLine(dest[1], dest[5], color);
	DebugLine::Get()->RenderLine(dest[2], dest[6], color);
	DebugLine::Get()->RenderLine(dest[3], dest[7], color);
}

bool OBBCollider::IsIntersect(Collider * other)
{
	if (bUsingCollider == false)
		return false;
	CollsionType type = other->GetCollisionType();
	bool result = false;
	switch (type)
	{
	case CollsionType::None:
		break;
	case CollsionType::OBB:
		result = CollisionOBB_OBB(this->obb, dynamic_cast<OBBCollider*>(other)->obb);
		break;
	case CollsionType::Sphere:
		break;
	case CollsionType::Capsule:
		break;
	default:
		break;
	}

	return false;
}

bool OBBCollider::IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist)
{
	return bUsingCollider ? RayInterSection(position, direction, MinRound, MaxRound, dist) : false;
}

void OBBCollider::SetObb()
{
	//쉐이더가 있으면 깊은 복사가 필요함.
	Transform temp;
	temp.World(transform->World());
	if (init != NULL)
		temp.World(init->World()*transform->World());

	temp.Position(&obb.Position);

	obb.AxisX = temp.Right();
	obb.AxisY = temp.Up();
	obb.AxisZ = temp.Forward();

	Vector3 scale;

	temp.Scale(&scale);
	obb.HalfSize = scale * 0.5f;

	MaxRound = obb.Position +
		obb.AxisX*obb.HalfSize.x +
		obb.AxisY*obb.HalfSize.y +
		obb.AxisZ*obb.HalfSize.z;
	MinRound = obb.Position -
		obb.AxisX*obb.HalfSize.x -
		obb.AxisY*obb.HalfSize.y -
		obb.AxisZ*obb.HalfSize.z;
}

bool OBBCollider::RayInterSection(Vector3& rayPos, Vector3 & rayDir, Vector3 & minV, Vector3 & maxV, float& dist)
{
	float d = 0.0f;
	float maxValue = FLT_MAX;
	Vector3 dir;
	D3DXVec3Normalize(&dir, &rayDir);
	if (Math::fABS(dir.x) < 0.0000001)
	{
		return false;
	}
	else
	{
		float inv = 1.0f / dir.x;
		float min = (minV.x - rayPos.x) * inv;
		float max = (maxV.x - rayPos.x) * inv;

		if (min > max)
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max(min, d);
		maxValue = Math::Min(max, maxValue);

		if (d > maxValue)
			return false;
	}

	if (Math::fABS(dir.y) < 0.0000001)
	{
		if (rayPos.y < minV.y || rayPos.y > maxV.y)
			return false;
	}
	else
	{
		float inv = 1.0f / dir.y;
		float min = (minV.y - rayPos.y) * inv;
		float max = (maxV.y - rayPos.y) * inv;

		if (min > max)
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max(min, d);
		maxValue = Math::Min(max, maxValue);

		if (d > maxValue)
			return false;
	}

	if (Math::fABS(dir.z) < 0.0000001)
	{
		if (rayPos.z < minV.z || rayPos.z > maxV.z)
			return false;
	}
	else
	{
		float inv = 1.0f / dir.z;
		float min = (minV.z - rayPos.z) * inv;
		float max = (maxV.z - rayPos.z) * inv;

		if (min > max)
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max(min, d);
		maxValue = Math::Min(max, maxValue);

		if (d > maxValue)
			return false;
	}
	dist = d;
	SelPos = rayPos + dir * d;
	return true;

}

bool OBBCollider::SperatingPlane(Vector3 position, Vector3 & direction, Obb & box1, Obb & box2)
{
	//축 겹침 검사
	/* 평면의 방정식(Ax+By+Cz+D = 0)을 응용? */
	/* 박스 중심간의 거리를 기준 축에 투영 시켜 구한 길이 */
	float val = fabsf(D3DXVec3Dot(&position, &direction));

	float val2 = 0.0f;
	/* box1의 각 축에서 기준 축에 투영시킨 거리의 반(중심으로 부터의 거리라서)을 합산 */
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisX * box1.HalfSize.x), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisY * box1.HalfSize.y), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box1.AxisZ * box1.HalfSize.z), &direction));
	/* box2의 각 축에서 기준 축에 투영시킨 거리의 반을 합산 */
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisX * box2.HalfSize.x), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisY * box2.HalfSize.y), &direction));
	val2 += fabsf(D3DXVec3Dot(&(box2.AxisZ * box2.HalfSize.z), &direction));

	return val > val2;
}

bool OBBCollider::CollisionOBB_OBB(Obb & box1, Obb & box2)
{
	/* 박스 중심간의 거리 */
	Vector3 position = box2.Position - box1.Position;

	/* 각 박스의 면 법선벡터로 얻은 축 */
	if (SperatingPlane(position, box1.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisZ, box1, box2) == true) return false;

	if (SperatingPlane(position, box2.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisZ, box1, box2) == true) return false;

	/* box1과 box2의 Edge 하나씩을 Cross시켜 얻은 축 */
	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisZ), box1, box2) == true) return false;

	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisZ), box1, box2) == true) return false;

	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisZ), box1, box2) == true) return false;

	return true;
}

//#include "Framework.h"
//#include "OBBCollider.h"
//
//OBBCollider::OBBCollider()
//	:Collider()
//{
//	Initalize();
//}
//
//OBBCollider::OBBCollider(Shader * shader, Shader * cs)
//	:Collider(shader,cs)
//{
//	Initalize();
//}
//
//OBBCollider::~OBBCollider()
//{
//}
//
//void OBBCollider::Initalize()
//{
//	lineCount = 12;
//
//	type = CollsionType::OBB;
//	
//	lines[0] = Vector3(-0.5f, -0.5f, -0.5f);
//	lines[1] = Vector3(-0.5f, +0.5f, -0.5f);
//	lines[2] = Vector3(+0.5f, -0.5f, -0.5f);
//	lines[3] = Vector3(+0.5f, +0.5f, -0.5f);
//	lines[4] = Vector3(-0.5f, -0.5f, +0.5f);
//	lines[5] = Vector3(-0.5f, +0.5f, +0.5f);
//	lines[6] = Vector3(+0.5f, -0.5f, +0.5f);
//	lines[7] = Vector3(+0.5f, +0.5f, +0.5f);
//
//	Super::Initalize();
//	CreateComputeBuffer();
//
//	rayCB = new ConstantBuffer(&rayDesc, sizeof(RayDesc));
//}
//
//void OBBCollider::Update()
//{	
//	for (int inst = 0; inst < colInfos.size(); inst++)
//	{
//		if (false == colInfos[inst]->bColliderOn) continue;
//
//		colInfos[inst]->transform->Update();
//
//		Matrix world = colInfos[inst]->transform->World();
//		colInfos[inst]->transform->Render();
//		if (colInfos[inst]->init != NULL)
//		{
//			colInfos[inst]->init->Update();
//			Matrix iWorld = colInfos[inst]->init->World();
//			colInfos[inst]->init->Render();
//			world = iWorld * world;
//		}
//		csInput[inst].data = world;
//	}
//}
//
//void OBBCollider::Render(const int& draw)
//{
//	if (false == bDebugMode)
//		return;
//	int loop = draw > 0 ? draw : colInfos.size();
//	for (int inst = 0; inst < loop; inst++)
//	{
//		//콜라이더 사용 off면 다음것
//		if (false == colInfos[inst]->bColliderOn)	continue;
//
//		Vector3 dest[8];
//		Matrix world = csInput[inst].data;
//
//		for (UINT i = 0; i < 8; i++)
//			D3DXVec3TransformCoord(&dest[i], &lines[i], &world);
//
//		//충돌 여부에 따라 컬러 설정
//		//Color color = ()?norColor:colColor;
//		Color color = norColor;
//
//		RenderLine(dest[0], dest[1], color);
//		RenderLine(dest[1], dest[3], color);
//		RenderLine(dest[3], dest[2], color);
//		RenderLine(dest[2], dest[0], color);
//
//
//		RenderLine(dest[4], dest[5], color);
//		RenderLine(dest[5], dest[7], color);
//		RenderLine(dest[7], dest[6], color);
//		RenderLine(dest[6], dest[4], color);
//
//
//		RenderLine(dest[0], dest[4], color);
//		RenderLine(dest[1], dest[5], color);
//		RenderLine(dest[2], dest[6], color);
//		RenderLine(dest[3], dest[7], color);
//	}
//
//	Super::Render();	
//}
//
//void OBBCollider::Property(const UINT & inst)
//{
//}
//
//
//void OBBCollider::RayIntersect(Vector3 & position, Vector3 & direction)
//{
//	rayDesc.position = position;
//	rayDesc.direction = direction;
//	rayCB->Apply();
//
//	csShader->AsConstantBuffer("CB_Ray")->SetConstantBuffer(rayCB->Buffer());
//
//	obbCSBuffer->UpdateInput();
//	csShader->AsSRV("Col_InputsA")->SetResource(obbCSBuffer->SRV());
//	csShader->AsUAV("OBB_Outputs")->SetUnorderedAccessView(obbCSBuffer->UAV());
//	csShader->Dispatch(1, 0, ceil(GetSize() / 1024.0f), 1, 1);
//	obbCSBuffer->Copy(csObbOutput, sizeof(CS_ObbOutputDesc) * MAX_COLLISION);
//}
//const Vector3 & OBBCollider::GetMinRound(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return csObbOutput[inst].MinRound;
//	return Vector3();
//}
//const Vector3 & OBBCollider::GetMaxRound(const UINT & inst)
//{
//	if (inst < colInfos.size())
//		return csObbOutput[inst].MaxRound;
//	return Vector3();
//}
//
//const bool & OBBCollider::IsIntersectRay(OUT float& dist, const UINT & inst)
//{
//	if (inst < colInfos.size())
//	{
//		dist = csObbOutput[inst].Dist;
//		return csObbOutput[inst].Intersect;
//	}
//	return false;
//}
//
//void OBBCollider::CreateComputeBuffer()
//{
//	Super::CreateComputeBuffer();
//	UINT outSize = MAX_COLLISION;
//	//csInput = new CS_InputDesc[outSize];
//	csObbOutput = new CS_ObbOutputDesc[outSize];
//
//	obbCSBuffer = new StructuredBuffer
//	(
//		csInput,
//		sizeof(CS_InputDesc), outSize,
//		true,
//		sizeof(CS_ObbOutputDesc), outSize
//	);
//}
