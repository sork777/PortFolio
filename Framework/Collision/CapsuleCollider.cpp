#include "Framework.h"
#include "CapsuleCollider.h"



CapsuleCollider::CapsuleCollider()
	:Collider()
{
}

CapsuleCollider::CapsuleCollider(Shader * shader, Shader * cs)
	:Collider(shader, cs)
{
}


CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::Update()
{
}

void CapsuleCollider::Render(const int& draw)
{
}

void CapsuleCollider::CreateComputeBuffer()
{
}
