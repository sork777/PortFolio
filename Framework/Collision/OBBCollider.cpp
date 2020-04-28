#include "Framework.h"
#include "OBBCollider.h"

OBBCollider::OBBCollider()
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
}

void OBBCollider::Update()
{	
	for (int inst = 0; inst < cmColInfos.size(); inst++)
	{
		if (false == cmColInfos[inst].bColliderOn) continue;

		cmColInfos[inst].transform->Update();
		if (cmColInfos[inst].init != NULL)
			cmColInfos[inst].init->Update();
		//SetObb(inst);
	}
}

void OBBCollider::Render()
{
	if (false == bDebugMode)
		return;
	for (int inst = 0; inst < cmColInfos.size(); inst++)
	{
		//콜라이더 사용 off면 다음것
		if (false == cmColInfos[inst].bColliderOn)	continue;

		Vector3 dest[8];
		Matrix world = cmColInfos[inst].transform->World();
		cmColInfos[inst].transform->Render();
		if (cmColInfos[inst].init != NULL)
		{
			Matrix iWorld = cmColInfos[inst].init->World();
			cmColInfos[inst].init->Render();
			world = iWorld*world;
		}

		for (UINT i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&dest[i], &lines[i], &world);

		//충돌 여부에 따라 컬러 설정
		//Color color = ()?norColor:colColor;
		Color color = norColor;

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

//TODO: 나중에 CS로 변경할것
bool OBBCollider::IsIntersect(Collider * other, const UINT & inst, const UINT & oinst)
{
	// 각 인스턴싱 넘버의 사이즈가 안맞음.
	if (inst >= cmColInfos.size()) return false;
	if (oinst >= other->GetSize()) return false;

	// 콜라이더 사용 여부
	if (false == cmColInfos[inst].bColliderOn)
		return false;
	CmCol_Info oInfo = other->GetCmInfo(oinst);
	if (false == oInfo.bColliderOn)
		return false;


	CollsionType type = other->GetCollisionType();
	bool result = false;
	switch (type)
	{
	case CollsionType::None:
		break;
	case CollsionType::OBB:
		result = CollisionOBB_OBB(obbInfo[inst].obb, dynamic_cast<OBBCollider*>(other)->obbInfo[inst].obb);
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

bool OBBCollider::IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist, const UINT & inst)
{
	if (inst >= cmColInfos.size()) return false;

	return cmColInfos[inst].bColliderOn ? RayInterSection(position, direction, obbInfo[inst].MinRound, obbInfo[inst].MaxRound, dist) : false;
}
//
//void OBBCollider::SetObb(const UINT& inst)
//{
//	//쉐이더가 있으면 깊은 복사가 필요함.
//	Transform temp;
//	Matrix world = cmColInfos[inst].transform->World();
//	cmColInfos[inst].transform->Render();
//	
//	if (cmColInfos[inst].init != NULL)
//	{
//		Matrix iWorld = cmColInfos[inst].init->World();
//		cmColInfos[inst].init->Render();
//		world = iWorld * world;
//	}
//	temp.World(world);
//
//	Obb obb = obbInfo[inst].obb;
//
//	temp.Position(&obb.Position);
//
//	obb.AxisX = temp.Right();
//	obb.AxisY = temp.Up();
//	obb.AxisZ = temp.Forward();
//
//	Vector3 scale;
//
//	temp.Scale(&scale);
//	obb.HalfSize = scale * 0.5f;
//
//	obbInfo[inst].MaxRound = obb.Position +
//		obb.AxisX*obb.HalfSize.x +
//		obb.AxisY*obb.HalfSize.y +
//		obb.AxisZ*obb.HalfSize.z;
//	obbInfo[inst].MinRound = obb.Position -
//		obb.AxisX*obb.HalfSize.x -
//		obb.AxisY*obb.HalfSize.y -
//		obb.AxisZ*obb.HalfSize.z;
//}

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
//
//bool OBBCollider::SperatingPlane(Vector3 position, Vector3 & direction, Obb & box1, Obb & box2)
//{
//	//축 겹침 검사
//	/* 평면의 방정식(Ax+By+Cz+D = 0)을 응용? */
//	/* 박스 중심간의 거리를 기준 축에 투영 시켜 구한 길이 */
//	float val = fabsf(D3DXVec3Dot(&position, &direction));
//
//	float val2 = 0.0f;
//	/* box1의 각 축에서 기준 축에 투영시킨 거리의 반(중심으로 부터의 거리라서)을 합산 */
//	val2 += fabsf(D3DXVec3Dot(&(box1.AxisX * box1.HalfSize.x), &direction));
//	val2 += fabsf(D3DXVec3Dot(&(box1.AxisY * box1.HalfSize.y), &direction));
//	val2 += fabsf(D3DXVec3Dot(&(box1.AxisZ * box1.HalfSize.z), &direction));
//	/* box2의 각 축에서 기준 축에 투영시킨 거리의 반을 합산 */
//	val2 += fabsf(D3DXVec3Dot(&(box2.AxisX * box2.HalfSize.x), &direction));
//	val2 += fabsf(D3DXVec3Dot(&(box2.AxisY * box2.HalfSize.y), &direction));
//	val2 += fabsf(D3DXVec3Dot(&(box2.AxisZ * box2.HalfSize.z), &direction));
//
//	return val > val2;
//}
//
//bool OBBCollider::CollisionOBB_OBB(Obb & box1, Obb & box2)
//{
//	/* 박스 중심간의 거리 */
//	Vector3 position = box2.Position - box1.Position;
//
//	/* 각 박스의 면 법선벡터로 얻은 축 */
//	if (SperatingPlane(position, box1.AxisX, box1, box2) == true) return false;
//	if (SperatingPlane(position, box1.AxisY, box1, box2) == true) return false;
//	if (SperatingPlane(position, box1.AxisZ, box1, box2) == true) return false;
//
//	if (SperatingPlane(position, box2.AxisX, box1, box2) == true) return false;
//	if (SperatingPlane(position, box2.AxisY, box1, box2) == true) return false;
//	if (SperatingPlane(position, box2.AxisZ, box1, box2) == true) return false;
//
//	/* box1과 box2의 Edge 하나씩을 Cross시켜 얻은 축 */
//	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisX), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisY), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisX, box2.AxisZ), box1, box2) == true) return false;
//
//	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisX), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisY), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisY, box2.AxisZ), box1, box2) == true) return false;
//								 
//	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisX), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisY), box1, box2) == true) return false;
//	if (SperatingPlane(position, Math::Cross(box1.AxisZ, box2.AxisZ), box1, box2) == true) return false;
//
//	return true;
//}

void OBBCollider::ComputeCollider()
{
}

void OBBCollider::CreateComputeBuffer()
{
	UINT outSize = MAX_COLLISION;
	csInput = new CS_ObbInputDesc[outSize];
	csOutput = new CS_ObbOutputDesc[outSize];

	computeBuffer = new StructuredBuffer
	(
		csInput,
		sizeof(CS_ObbInputDesc), outSize,
		true,
		sizeof(CS_ObbOutputDesc), outSize
	);
}
