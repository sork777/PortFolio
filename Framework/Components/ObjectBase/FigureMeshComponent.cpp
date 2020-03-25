#include "Framework.h"
#include "FigureMeshComponent.h"



FigureMeshComponent::FigureMeshComponent(MeshRender* mesh)
	:meshRender(mesh)
{
	componentName = L"FigureMeshComp";
	type = ObjectBaseComponentType::FigureMesh;
	//meshRender = new MeshRender(mesh);
}


FigureMeshComponent::~FigureMeshComponent()
{
	SafeDelete(meshRender);
}

void FigureMeshComponent::Update()
{
	meshRender->Update();
	Super::Update();
}

void FigureMeshComponent::Render()
{
	meshRender->Render();
	Super::Render();
}

bool FigureMeshComponent::Property()
{
	bool bChange = false;
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChange |= meshRender->GetMaterial()->Property();
	}
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChange |= baseTransform->Property();
	}
	bChange |= Super::Property();
	return bChange;
}

Transform * FigureMeshComponent::GetTransform(const UINT & instance)
{
	return meshRender->GetTransform(instance);
}

void FigureMeshComponent::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	meshRender->Tech(mesh);
	Super::Tech(mesh, model, anim);	
}

void FigureMeshComponent::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	meshRender->Pass(mesh);
	Super::Pass(mesh, model, anim);
}

void FigureMeshComponent::SetShader(Shader * shader)
{
	Super::SetShader(shader);
	meshRender->GetMesh()->SetShader(shader);
}

void FigureMeshComponent::AddInstanceData()
{
	int index = meshRender->GetInstSize();
	meshRender->AddInstance();
	meshRender->GetTransform(index)->Local(baseTransform);
	Super::AddInstanceData();
	meshRender->UpdateTransforms();
}

void FigureMeshComponent::DelInstanceData(const UINT & instance)
{
	meshRender->DelInstance(instance);
	Super::DelInstanceData(instance);
}
