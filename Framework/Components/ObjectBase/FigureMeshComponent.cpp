#include "Framework.h"
#include "FigureMeshComponent.h"



FigureMeshComponent::FigureMeshComponent(MeshRender* mesh)
	:meshRender(mesh)
{
	type = ObjectBaseComponentType::FigureMesh;
	//meshRender = new MeshRender(mesh);
}


FigureMeshComponent::~FigureMeshComponent()
{
	SafeDelete(meshRender);
}

void FigureMeshComponent::Update()
{
	Super::Update();
	meshRender->Update();
}

void FigureMeshComponent::Render()
{
	meshRender->Render();
}

bool FigureMeshComponent::Property(const UINT& instance)
{
	bool bChange = false;
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChange |= meshRender->GetMaterial()->Property();
	}
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChange |= meshRender->GetTransform(instance)->Property();
	}
	return bChange;
}

Transform * FigureMeshComponent::GetTransform(const UINT & instance)
{
	return meshRender->GetTransform(instance);
}

void FigureMeshComponent::AddInstanceData()
{
	meshRender->AddInstance();
}

void FigureMeshComponent::DelInstanceData(const UINT & instance)
{
	meshRender->DelInstance(instance);
}
