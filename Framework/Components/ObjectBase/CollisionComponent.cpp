#include "Framework.h"
#include "CollisionComponent.h"



CollisionComponent::CollisionComponent()
{
	type = ObjectBaseComponentType::Collision;
}


CollisionComponent::~CollisionComponent()
{
}

void CollisionComponent::Update()
{
	Super::Update();
}

void CollisionComponent::Render()
{
}

bool CollisionComponent::Property(const UINT & instance)
{
	return false;
}

Transform * CollisionComponent::GetTransform(const UINT & instance)
{
	return nullptr;
}

void CollisionComponent::AddInstanceData()
{
}

void CollisionComponent::DelInstanceData(const UINT & instance)
{
}
