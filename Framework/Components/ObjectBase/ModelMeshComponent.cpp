#include "Framework.h"
#include "ModelMeshComponent.h"



ModelMeshComponent::ModelMeshComponent(Model* model)
	:meshRender(NULL), animation(NULL)
{
	componentName = L"ModelMeshComp";
	
	type = ObjectBaseComponentType::ModelMesh;

	//컴포넌트 마다 가지고있는 모델이 달라야 모델 바꿀때 다른게 안변함
	skeletonMesh = new Model(*model);
	meshRender = new ModelRender(skeletonMesh);
	if (skeletonMesh->IsAnimationModel() == true)
	{
		animation = new ModelAnimator(skeletonMesh);
	}
}

ModelMeshComponent::ModelMeshComponent(const ModelMeshComponent&  modelComp)
	:ObjectBaseComponent(modelComp)
	, meshRender(NULL), animation(NULL)
{

	componentName = L"ModelMeshComp";
	type = ObjectBaseComponentType::ModelMesh;
		
	skeletonMesh = new Model(*modelComp.skeletonMesh);
	ClonningComp(modelComp);
}

ModelMeshComponent::~ModelMeshComponent()
{
	SafeDelete(skeletonMesh);
	SafeDelete(meshRender);
	SafeDelete(animation);
	
}

void ModelMeshComponent::ClonningComp(const ModelMeshComponent & oComp)
{
	meshRender = new ModelRender(skeletonMesh);
	if (skeletonMesh->IsAnimationModel() == true)
	{
		ModelAnimator* oAnim = oComp.animation;
		animation = new ModelAnimator(skeletonMesh);
		// 복사 원본에 애니메이션이 있으면 클립데이터를 복사
		if (NULL != oAnim)
			animation->CloneClips(oAnim->Clips());
	}
	Super::ClonningChildren(oComp.children);
}

void ModelMeshComponent::CompileComponent(const ModelMeshComponent & OBComp)
{
	Super::CompileComponent(OBComp);

	SafeDelete(meshRender);
	SafeDelete(animation);

	// 메시데이터 교체
	skeletonMesh->ModelMeshChanger(*OBComp.skeletonMesh);
	ClonningComp(OBComp);
}

void ModelMeshComponent::Update()
{
	if (animation != NULL)
	{
		animation->Update();

		{
			int loop = GetInstSize();
			for (int i = 0; i < loop; i++)
				animation->PlayAnim(i);
		}
	}
	else if (meshRender != NULL)
	{
		meshRender->Update();
	}
	skeletonMesh->Update();
	
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
	skeletonMesh->Render();
	
	Super::Render();
}

bool ModelMeshComponent::Property(const int& instance)
{
	//인스턴스가 없으면 false
	if (0 >= GetInstSize())		return false;
	//인스턴스 번호가 갯수보다 크면 false
	if (0 >= GetInstSize()- instance)		return false;

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

		if (instance < 0)
		{
			if (GetTransform()->Property())
			{
				bChange = true;
				//로컬을 수정하는거니까 로컬만 받아야함.
				baseTransform->Local(GetTransform()->Local());
			}
			if (ImGui::Button("Reset"))
			{
				baseTransform->Local(baseInitTransform);
				GetTransform()->Local(baseTransform);
			}
		}
		else
		{
			if (GetTransform(instance)->Property())
			{
				chageTrans[instance] = true;
				bChange = true;
			}
			if (ImGui::Button("Reset"))
			{
				GetTransform(instance)->Local(baseInitTransform);
				chageTrans[instance] = false;
			}
		}
	}

	bChange |= Super::Property(instance);
	return bChange;
}

void ModelMeshComponent::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (animation != NULL)
	{
		skeletonMesh->Tech(anim);
	}
	else
		skeletonMesh->Tech(model);
	Super::Tech(mesh, model, anim);
}

void ModelMeshComponent::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (animation != NULL)
	{
		skeletonMesh->Pass(anim);
	}
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
	skeletonMesh->GetTransform(index)->Local(baseInitTransform);	
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

void ModelMeshComponent::SetAnimState(const AnimationState & state, const UINT & instance)
{
	if (NULL != animation)
		animation->SetAnimState(state, instance);
}

void ModelMeshComponent::PlayAllAnim()
{
	if (NULL != animation)
	{
		int loop = GetInstSize();
		for (int i = 0; i < loop; i++)
		animation->SetAnimState(AnimationState::Play, i);
	}
}
void ModelMeshComponent::StopAllAnim()
{
	if (NULL != animation)
	{
		int loop = GetInstSize();
		for (int i = 0; i < loop; i++)
			animation->SetAnimState(AnimationState::Stop, i);
	}
}
