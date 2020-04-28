#include "Framework.h"
#include "ModelMeshComponent.h"



ModelMeshComponent::ModelMeshComponent(Model* model)
	:meshRender(NULL), animation(NULL)
	,skeletonMesh(model)
{
	componentName = L"ModelMeshComp";
	type = ObjectBaseComponentType::ModelMesh;
	
	meshRender = new ModelRender(model);
	if (model->IsAnimationModel() == true)
	{
		animation = new ModelAnimator(model);
	}
}


ModelMeshComponent::~ModelMeshComponent()
{
	SafeDelete(skeletonMesh);
	SafeDelete(meshRender);
	SafeDelete(animation);
	
}

void ModelMeshComponent::Update()
{
	if (animation != NULL)
	{
		animation->Update();

		//TODO: 플레이 선택에 따라 변경해야함.
		int loop = bEditMode ? 1 : GetInstSize();
		for (int i = 0; i < loop; i++)
			animation->PlayAnim(i);
	}
	else if (meshRender != NULL)
	{
		meshRender->Update();
	}

	Super::Update();
}

void ModelMeshComponent::Render()
{
	int inst = bEditMode ? 1 : -1;

	if (animation != NULL)
	{
		animation->Render(inst);
	}
	else if (meshRender != NULL)
	{
		meshRender->Render(inst);
	}
	Super::Render();
}

bool ModelMeshComponent::Property(const UINT& instance)
{
	if (GetInstSize() <= instance)
		return false;
	bool bChange = false;
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (Material* mat : skeletonMesh->Materials())
		{
			bChange |= mat->Property();
			ImGui::Separator();
		}
	}
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (GetTransform(instance)->Property())
		{
			chageTrans[instance] = true;
			bChange = true;
		}
		if (ImGui::Button("Reset"))
		{
			GetTransform(instance)->Local(baseTransform);
			chageTrans[instance] = false;
		}
	}

	bChange |= Super::Property();
	return bChange;
}

void ModelMeshComponent::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (animation != NULL)
		skeletonMesh->Tech(anim);
	else
		skeletonMesh->Tech(model);

	Super::Tech(mesh, model, anim);
}

void ModelMeshComponent::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (animation != NULL)
		skeletonMesh->Pass(anim);
	else
		skeletonMesh->Pass(model);

	Super::Pass(mesh, model, anim);
}

void ModelMeshComponent::SetShader(Shader * shader)
{
	Super::SetShader(shader);
	if (animation != NULL)
	{
		animation->SetShader(shader);
	}
	else if (meshRender != NULL)
	{
		meshRender->SetShader(shader);
	}
}

void ModelMeshComponent::AddInstanceData()
{
	int index = skeletonMesh->GetInstSize();
	skeletonMesh->AddInstance();
	skeletonMesh->GetTransform(index)->Local(baseTransform);	
	Super::AddInstanceData();
	skeletonMesh->UpdateTransforms();
}

void ModelMeshComponent::DelInstanceData(const UINT& instance)
{
	skeletonMesh->DelInstance(instance);
	Super::DelInstanceData(instance);
}

const UINT & ModelMeshComponent::GetInstSize()
{
	return skeletonMesh->GetInstSize();
}

Transform * ModelMeshComponent::GetTransform(const UINT & instance)
{
	return skeletonMesh->GetTransform(instance);
}
