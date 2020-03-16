#include "Framework.h"
#include "ObjectBaseComponent.h"

ObjectBaseComponent::ObjectBaseComponent()
{
}

ObjectBaseComponent::~ObjectBaseComponent()
{
	SafeDelete(parent);
	children.clear();
	children.shrink_to_fit();
}

void ObjectBaseComponent::Update()
{
	// 모델 메시 아니면 소켓이 필요 없음.
	// 매번 위치 바꿔줄 필요도 없음.
	if (parent->GetType() != ObjectBaseComponentType::ModelMesh)
	{
		//부모를 다이나믹 형변환해서 컴포넌트 추출
		ModelMeshComponent* modelMesh = dynamic_cast<ModelMeshComponent*>(parent);
		// 모델과 애니메이터 추출
		ModelAnimator* animator = modelMesh->GetAnimation();
		Model* model = animator->GetModel();
		//같은 오브젝트면 소켓도 같음
		if (parentSocket >= 0)
		{
			// 인스턴스만큼 위치 업뎃.
			UINT instCount = model->GetInstSize();
			for (UINT i = 0; i < instCount; i++)
			{
				Matrix world = model->GetTransform(i)->World();
				Matrix attach = animator->GetboneWorld(i, parentSocket);
				attach *= world;
				GetTransform(i)->Parent(attach);
			}
		}
	}
}

#pragma region 부모자식 링크/언링크
void ObjectBaseComponent::LinkParentComponent(ObjectBaseComponent * component, const UINT& instance)
{
	parent = component;
	
	//위치 조정
	GetTransform(instance)->Parent(component->GetTransform(instance));
	
}

void ObjectBaseComponent::UnlinkParentComponent(const UINT& instance)
{
	
	GetTransform(instance)->Parent(NULL);
	
	parent = NULL;
}

void ObjectBaseComponent::LinkChildComponent(ObjectBaseComponent * component, const UINT& instance)
{
	children.emplace_back(component);
	component->LinkParentComponent(this, instance);
}

void ObjectBaseComponent::UnlinkChildComponent(ObjectBaseComponent * component, const UINT& instance)
{
	int index = 0;

	for (ObjectBaseComponent* child : children)
	{
		if (child == component)
		{
			child->UnlinkParentComponent(instance);
			children.erase(children.begin() + index);
			break;
		}
		index++;
	}
}
#pragma endregion
