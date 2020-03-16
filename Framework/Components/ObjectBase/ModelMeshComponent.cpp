#include "Framework.h"
#include "ModelMeshComponent.h"



ModelMeshComponent::ModelMeshComponent(Model* model)
	:meshRender(NULL), animation(NULL)
	,skeletonMesh(model)
{
	type = ObjectBaseComponentType::ModelMesh;
	if (model->IsAnimationModel() == true)
	{
		//ModelAnimator* animator = new ModelAnimator(model);
		//animation = new AnimationComponent(animator);
		animation = new ModelAnimator(model);
		//animation = new AnimationComponent(animator);
	}
	else
		meshRender = new ModelRender(model);
}


ModelMeshComponent::~ModelMeshComponent()
{
	SafeDelete(skeletonMesh);
	SafeDelete(meshRender);
	SafeDelete(animation);
	
}

void ModelMeshComponent::Update()
{
	Super::Update();

	if (meshRender != NULL)
	{
		meshRender->Update();
	}
	else if (animation != NULL)
		animation->Update();
}

void ModelMeshComponent::Render()
{
	if (meshRender != NULL)
	{
		meshRender->Render();
	}
	else if (animation != NULL)
		animation->Render();
}

bool ModelMeshComponent::Property(const UINT& instance)
{
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
		bChange |= skeletonMesh->GetTransform(instance)->Property();
	}
	return bChange;
}

Transform * ModelMeshComponent::GetTransform(const UINT & instance)
{
	return skeletonMesh->GetTransform(instance);
}

void ModelMeshComponent::AddInstanceData()
{
	skeletonMesh->AddInstance();
}

void ModelMeshComponent::DelInstanceData(const UINT& instance)
{
	skeletonMesh->DelInstance(instance);
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
