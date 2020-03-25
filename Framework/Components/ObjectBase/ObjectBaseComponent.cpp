#include "Framework.h"
#include "ObjectBaseComponent.h"

ObjectBaseComponent::ObjectBaseComponent()
	:shader(NULL), parent(NULL)
{
	baseTransform = new Transform();
}

ObjectBaseComponent::~ObjectBaseComponent()
{
	SafeDelete(baseTransform);
	
	if (NULL == parent)
	{
		// Root ������Ʈ�϶�. Root�� �����Ǹ鼭 ���� �����ɶ�.
		for (ObjectBaseComponent* child : children)
		{
			//���������� ��ũ ���� �����ϱ�
			child->UnlinkParentComponent();
			SafeDelete(child);
		}
	}
	else
	{
		// ������ ���� ������Ʈ�� ���ϵ� �ѱ��
		for (ObjectBaseComponent* child : children)
			parent->LinkChildComponent(child);
	}
	children.clear();
	children.shrink_to_fit();
	SafeDelete(shader);
}

void ObjectBaseComponent::Update()
{
	if (NULL == parent)
	{

	}
	// �� �޽� �ƴϸ� ������ �ʿ� ����.
	// �Ź� ��ġ �ٲ��� �ʿ䵵 ����.

	else if (parent->GetType() == ObjectBaseComponentType::ModelMesh)
	{
		//�θ� ���̳��� ����ȯ�ؼ� ������Ʈ ����
		ModelMeshComponent* modelMesh = dynamic_cast<ModelMeshComponent*>(parent);
		// �𵨰� �ִϸ����� ����
		ModelAnimator* animator = modelMesh->GetAnimation();
		//���� ������Ʈ�� ���ϵ� ����
		
		if (NULL!= animator && parentSocket >= 0)
		{
			// �ν��Ͻ���ŭ ��ġ ����.
			UINT instCount = modelMesh->GetInstSize();
			for (UINT i = 0; i < instCount; i++)
			{
				Matrix world = modelMesh->GetTransform(i)->World();
				Matrix attach = animator->GetboneWorld(i, parentSocket);
				attach *= world;

				this->GetTransform(i)->Parent(attach);
			}
		}
	}
	//��Ʈ�� ������Ʈ ȣ���ϸ� �ڽ� �ڵ� ȣ���ϰ�
	for (ObjectBaseComponent* child : children)
		child->Update();
}

void ObjectBaseComponent::Render()
{
	for (ObjectBaseComponent* child : children)
		child->Render();
}

bool ObjectBaseComponent::Property()
{
	bool bChange = false;

	ImGui::Text("ParentBone : %s", String::ToString( parentSocketName).c_str());

	if (NULL == parent)
	{

	}
	else if (parent->GetType() == ObjectBaseComponentType::ModelMesh)
	{
		ModelMeshComponent* modelMesh = dynamic_cast<ModelMeshComponent*>(parent);
		Model* model = modelMesh->GetMesh();

		
		int click = -1;
		int presocket = parentSocket;
		if (ImGui::CollapsingHeader("SelectBone"))
		{
			parentSocket = model->BoneHierarchy(&click);
		}
			
		bChange |= presocket != parentSocket;
		if (true == bChange)
		{
			if (parentSocket < 0)
				parentSocketName = L"None";
			else
				parentSocketName = model->BoneByIndex(parentSocket)->Name();
		}
	}
	return bChange;
}

void ObjectBaseComponent::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (ObjectBaseComponent* child : children)
		child->Tech(mesh,model,anim);
}

void ObjectBaseComponent::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (ObjectBaseComponent* child : children)
		child->Pass(mesh, model, anim);
}

void ObjectBaseComponent::SetShader(Shader * shader)
{
	this->shader = shader;
	//for (ObjectBaseComponent* child : children)
	//	child->SetShader(shader);
}

void ObjectBaseComponent::AddInstanceData()
{
	instancingCount++;
	for (ObjectBaseComponent* child : children)
		child->AddInstanceData();
}

void ObjectBaseComponent::DelInstanceData(const UINT & instance)
{
	if(instancingCount>0)
		instancingCount--;

	for (ObjectBaseComponent* child : children)
		child->DelInstanceData(instance);
}

void ObjectBaseComponent::ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	//������ ��������
	ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
	//������ ������
	ImVec2 wSize = ImGui::GetWindowSize();
	if (wSize.y < pos.y) return;
	if (wSize.x < pos.x) return;

	ImGuiTreeNodeFlags flags = children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (this == *selectedComp)
		flags |= ImGuiTreeNodeFlags_Selected;
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

	if (ImGui::TreeNodeEx(String::ToString(componentName).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			*selectedComp = this;
		}
		if (ImGui::IsItemClicked(1))
		{
			*selectedComp = this;
			ImGui::OpenPopup("AddComponentPopup");
		}
		ComponentPopup();
		for (auto& child : children)
		{
			child->ComponentHeirarchy(selectedComp);
		}
		ImGui::TreePop();
	}
	ImGui::PopStyleVar();
}

void ObjectBaseComponent::ComponentPopup()
{
	bool boo = false;
	int itemIdx = -1;
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		if (ImGui::BeginMenu("SkeletonMeshComponent"))
		{
			AddSkeletonMeshComponentPopup();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("FigureMeshComponent"))
		{
			AddFigureMeshComponentPopup();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("CollisionComponent"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("CameraComponent"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
}

void ObjectBaseComponent::AddSkeletonMeshComponentPopup()
{
	//TODO : ���� �Ŵ����� ��ϵ� SkeletonMesh���� ������Ʈ �����ؼ� �ޱ�
	ModelAsset* asset =dynamic_cast<ModelAsset*>( AssetManager::Get()->ViewAssets(ContentsAssetType::Model));

	if (NULL != asset)
	{
		LinkChildComponent(asset->GetModelMeshCompFromModelAsset());
	}
}

void ObjectBaseComponent::AddFigureMeshComponentPopup()
{
	//TODO : ���� �Ŵ����� ��ϵ� FigureMesh���� ������Ʈ �����ؼ� �ޱ�
}

void ObjectBaseComponent::AddCollisionComponentPopup()
{
	//TODO : CollisionType �����ϱ�.
}


#pragma region �θ��ڽ� ��ũ/��ũ
void ObjectBaseComponent::LinkParentComponent(ObjectBaseComponent * component)
{
	parent = component;

	//������Ʈ�� �ν��Ͻ� ���� ���ߴ°� ���⼭ �Ұ���.
	UINT parentInst = component->GetInstSize();
	UINT maxLoop = parentInst > instancingCount ? parentInst : instancingCount;
	for (UINT i = 0; i < maxLoop; i++)
	{
		//�θ� ������Ʈ�� �ν��Ͻ� ������ �� ����
		if (parentInst - 1 < i)
		{
			component->AddInstanceData();
		}
		//�ڽ� ������Ʈ�� �ν��Ͻ� ������ �� ����
		else if (instancingCount < i + 1)
		{
			AddInstanceData();
		}

		//��ġ ����
		GetTransform(i)->Parent(component->GetTransform(i));
	}
	
}

void ObjectBaseComponent::UnlinkParentComponent()
{
	parent = NULL;

	// �̹� Link�Ҷ� ���� �������� �ν��Ͻ� ������ ���缭 ������ ���ָ� �ɰ�.
	for (UINT i = 0; i < instancingCount; i++)
	{
		GetTransform(i)->Parent(NULL);
	}
}

void ObjectBaseComponent::LinkChildComponent(ObjectBaseComponent * component)
{
	children.emplace_back(component);
	//�ڽ��� ���̴��� �ڽ��� ���̴��� ����
	component->SetShader(shader);
	component->LinkParentComponent(this);
}

void ObjectBaseComponent::UnlinkChildComponent(ObjectBaseComponent * component)
{
	int index = 0;

	for (ObjectBaseComponent* child : children)
	{
		if (child == component)
		{
			child->UnlinkParentComponent();
			children.erase(children.begin() + index);
			break;
		}
		index++;
	}
}
#pragma endregion
