#include "Framework.h"
#include "CameraComponent.h"



CameraComponent::CameraComponent()
{
	type = ObjectBaseComponentType::Camera;
}


CameraComponent::~CameraComponent()
{
}

void CameraComponent::Update()
{
	Super::Update();
}

void CameraComponent::Render()
{
}

bool CameraComponent::Property(const UINT & instance)
{
	return false;
}

Transform * CameraComponent::GetTransform(const UINT & instance)
{
	return nullptr;
}
