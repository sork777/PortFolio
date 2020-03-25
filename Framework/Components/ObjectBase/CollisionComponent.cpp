#include "Framework.h"
#include "CollisionComponent.h"



CollisionComponent::CollisionComponent()
{
	componentName = L"CollisionComp";
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
	Super::Render();
}

bool CollisionComponent::Property()
{
	return false;
}

Transform * CollisionComponent::GetTransform(const UINT & instance)
{
	return nullptr;
}

void CollisionComponent::AddInstanceData()
{
	Super::AddInstanceData();
}

void CollisionComponent::DelInstanceData(const UINT & instance)
{
	Super::DelInstanceData(instance);
}
