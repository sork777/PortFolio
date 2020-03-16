#include "Framework.h"
#include "CapsuleCollider.h"



CapsuleCollider::CapsuleCollider()
{
}


CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::Update()
{
}

void CapsuleCollider::Render(Color color)
{
}

bool CapsuleCollider::IsIntersect(Collider * other)
{
	return false;
}

bool CapsuleCollider::IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist)
{
	return false;
}
