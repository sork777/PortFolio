#include "Framework.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider()
{
}

SphereCollider::~SphereCollider()
{
}

void SphereCollider::Update()
{
}

void SphereCollider::Render(Color color)
{
}

bool SphereCollider::IsIntersect(Collider * other)
{
	return false;
}

bool SphereCollider::IsIntersectRay(Vector3 & position, Vector3 & direction, float & dist)
{
	return false;
}
