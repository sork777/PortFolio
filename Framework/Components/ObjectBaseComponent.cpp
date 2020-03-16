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
	// �� �޽� �ƴϸ� ������ �ʿ� ����.
	// �Ź� ��ġ �ٲ��� �ʿ䵵 ����.
	if (parent->GetType() != ObjectBaseComponentType::ModelMesh)
	{
		//�θ� ���̳��� ����ȯ�ؼ� ������Ʈ ����
		ModelMeshComponent* modelMesh = dynamic_cast<ModelMeshComponent*>(parent);
		// �𵨰� �ִϸ����� ����
		ModelAnimator* animator = modelMesh->GetAnimation();
		Model* model = animator->GetModel();
		//���� ������Ʈ�� ���ϵ� ����
		if (parentSocket >= 0)
		{
			// �ν��Ͻ���ŭ ��ġ ����.
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

#pragma region �θ��ڽ� ��ũ/��ũ
void ObjectBaseComponent::LinkParentComponent(ObjectBaseComponent * component, const UINT& instance)
{
	parent = component;
	
	//��ġ ����
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
