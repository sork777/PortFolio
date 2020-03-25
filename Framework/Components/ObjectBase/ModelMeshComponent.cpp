#include "Framework.h"
#include "ModelMeshComponent.h"



ModelMeshComponent::ModelMeshComponent(Model* model)
	:meshRender(NULL), animation(NULL)
	,skeletonMesh(model)
{
	componentName = L"ModelMeshComp";
	type = ObjectBaseComponentType::ModelMesh;
	// 버튼용으로 필요는 함.
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
		for (int i = 0; i < GetInstSize(); i++)
			PlayAnim(i);
	}
	else if (meshRender != NULL)
	{
		meshRender->Update();
	}

	Super::Update();
}

void ModelMeshComponent::Render()
{
	if (animation != NULL)
	{
		animation->Render();
	}
	else if (meshRender != NULL)
	{
		meshRender->Render();
	}
	Super::Render();
}

bool ModelMeshComponent::Property()
{
	if (skeletonMesh->GetInstSize() < 1)
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
		if (baseTransform->Property())
		{
			//TODO: 나중에 에디터의 컴파일로 옮기기
			for (UINT i = 0; i < GetInstSize(); i++)
			{
				skeletonMesh->GetTransform(i)->Local(baseTransform);
			}
			bChange = true;
		}
	}

	bChange |= Super::Property();
	return bChange;
}

Transform * ModelMeshComponent::GetTransform(const UINT & instance)
{
	return skeletonMesh->GetTransform(instance);
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


inline void ModelMeshComponent::PlayAnim(const UINT & instance)
{
	if (animation != NULL)
		animation->PlayAnim(instance);
}

inline void ModelMeshComponent::PlayClip(const UINT & instance, const UINT & clip, const float & speed, const float & takeTime)
{
	if (animation != NULL)
		animation->PlayClip(instance, clip, speed, takeTime);
}
