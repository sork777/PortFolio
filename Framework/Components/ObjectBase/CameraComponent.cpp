#include "Framework.h"
#include "CameraComponent.h"



CameraComponent::CameraComponent()
{
	componentName=L"CameraComp";
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
	Super::Render();
}

bool CameraComponent::Property(const int& instance)
{
	return false;
}

Transform * CameraComponent::GetTransform(const UINT & instance)
{
	return nullptr;
}
