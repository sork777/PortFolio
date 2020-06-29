#include "Framework.h"
#include "ModelMeshComponent.h"



ModelMeshComponent::ModelMeshComponent(Model* model)
	:meshRender(NULL), animation(NULL)
{
	componentName = L"ModelMeshComp";
	
	std::wcout << componentName << " 생성" << endl;
	type = ObjectBaseComponentType::ModelMesh;

	//컴포넌트 마다 가지고있는 모델이 달라야 모델 바꿀때 다른게 안변함
	skeletonMesh = new Model(*model);
	meshRender = new ModelRender(skeletonMesh);
	if (skeletonMesh->IsAnimationModel() == true)
	{
		animation = new ModelAnimator(skeletonMesh);
	}
	//std::cout << "모델 주소 : " << skeletonMesh << endl;
	//std::cout << "Render 주소 : " << meshRender << endl;
	//std::cout << "Anim 주소 : " << animation << endl;
}

ModelMeshComponent::ModelMeshComponent(const ModelMeshComponent&  modelComp)
	:ObjectBaseComponent(modelComp)
	, meshRender(NULL), animation(NULL)
{
	std::wcout << componentName << " 복사생성" << endl;

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

	//std::wcout << componentName << "-Clonning Comp 함수진입 " << endl;
	//std::cout << "원본 컴포넌트 주소 : " << &oComp << endl;
	//std::cout << "현재 컴포넌트 주소 : " << this << endl;
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
	//std::wcout << componentName << "-Clonning Comp 함수탈출 " << endl;
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

		//TODO: 플레이 선택에 따라 변경해야함.
		//if (!bEditMode)
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
