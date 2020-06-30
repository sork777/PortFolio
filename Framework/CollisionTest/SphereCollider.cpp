#include "Framework.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider()
	:Collider()
{
}

SphereCollider::SphereCollider(Shader * shader, Shader * cs)
	:Collider(shader, cs)
{
}
SphereCollider::~SphereCollider()
{
}

void SphereCollider::Update()
{
}

void SphereCollider::Render(const int& draw)
{
}

void SphereCollider::CreateComputeBuffer()
{
}
