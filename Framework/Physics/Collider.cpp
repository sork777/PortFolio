#include "Framework.h"
#include "Collider.h"



Collider::Collider(Transform * transform, Transform * init)
	:transform(transform), init(init), SelPos(-1, -1, -1)
	, bUsingCollider(true)
{	
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


Collider::~Collider()
{
}

bool Collider::IsIntersect(Collider * other)
{
	return bUsingCollider?Collision(this->obb,other->obb):false;
}

bool Collider::IsIntersect(Vector3& position, Vector3 & direction, float& dist)
{
	return bUsingCollider?RayInterSection(position,direction,MinRound,MaxRound,dist):false;
}

void Collider::Update()
{
	if (bUsingCollider == false)
		return;

	SetObb();
}

void Collider::Render(Color color)
{
	if (bUsingCollider == false)
		return;

	Vector3 dest[8];
	Matrix world = transform->World();
	if (init != NULL)
		world = init->World()*transform->World();

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

void Collider::SetObb()
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
		obb.AxisX*obb.HalfSize.x+
		obb.AxisY*obb.HalfSize.y+
		obb.AxisZ*obb.HalfSize.z;
	MinRound = obb.Position -
		obb.AxisX*obb.HalfSize.x -
		obb.AxisY*obb.HalfSize.y -
		obb.AxisZ*obb.HalfSize.z;
}

bool Collider::RayInterSection(Vector3& rayPos, Vector3 & rayDir, Vector3 & minV, Vector3 & maxV,float& dist)
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

		if( min > max )
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max( min, d );
		maxValue = Math::Min( max, maxValue );

		if( d > maxValue )
			return false;
	}

	if( Math::fABS( dir.y ) < 0.0000001 )
	{
		if( rayPos.y < minV.y || rayPos.y > maxV.y )
			return false;
	}
	else
	{
		float inv = 1.0f / dir.y;
		float min = (minV.y - rayPos.y) * inv;
		float max = (maxV.y - rayPos.y) * inv;

		if( min > max )
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max( min, d );
		maxValue = Math::Min( max, maxValue );

		if( d > maxValue )
			return false;
	}

	if( Math::fABS( dir.z ) < 0.0000001 )
	{
		if( rayPos.z < minV.z || rayPos.z > maxV.z )
			return false;
	}
	else
	{
		float inv = 1.0f / dir.z;
		float min = (minV.z - rayPos.z) * inv;
		float max = (maxV.z - rayPos.z) * inv;

		if( min > max )
		{
			float temp = min;
			min = max;
			max = temp;
		}

		d = Math::Max( min, d );
		maxValue = Math::Min( max, maxValue );

		if( d > maxValue )
			return false;
	}
	dist = d;
	SelPos = rayPos + dir * d;
	return true;
	
}


bool Collider::SperatingPlane(Vector3 position, Vector3 & direction, Obb & box1, Obb & box2)
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

bool Collider::Collision(Obb & box1, Obb & box2)
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
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisX, box2.AxisZ), box1, box2) == true) return false;

	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisY, box2.AxisZ), box1, box2) == true) return false;

	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, Cross(box1.AxisZ, box2.AxisZ), box1, box2) == true) return false;
	
	return true;
}

Vector3 Collider::Cross(Vector3 & vec1, Vector3 & vec2)
{
	float x = vec1.y*vec2.z - vec1.z*vec2.y;
	float y = vec1.z*vec2.x - vec1.x*vec2.z;
	float z = vec1.x*vec2.y - vec1.y*vec2.x;

	return Vector3(x, y, z);
}
