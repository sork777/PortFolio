#include "Framework.h"
#include "ObjectBaseComponent.h"

ObjectBaseComponent::ObjectBaseComponent()
	:shader(NULL), parent(NULL)
{
	baseTransform = new Transform();
	compID = GUID_Generator::Generate();

	std::cout << "���� ������Ʈ �ּ� : " << this << endl;
}

ObjectBaseComponent::ObjectBaseComponent(const ObjectBaseComponent& OBComp)
	:shader(NULL), parent(NULL)
{
	bEditMode = true;
	shader = OBComp.shader;
	compID = OBComp.compID;
	parentSocketName = OBComp.parentSocketName;
	componentName = OBComp.componentName;
	parentSocket = OBComp.parentSocket;

	baseTransform = new Transform();
	baseTransform->Local(OBComp.baseTransform->Local());
	//���̽��� �θ� Ʈ�������� ���� �ʴ´�.
	baseTransform->Parent(OBComp.baseTransform->Parent());

	std::cout << "���� ������Ʈ �ּ� : " << &OBComp << endl;
	std::cout << "���� ������Ʈ �ּ� : " << this << endl;

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
	//���̴��� �ۿ��� ���°Ŷ� ������ �н�
//	
}


/*
	1. ������ ���Ϳ��� ������Ʈ�� �޾Ƽ� �����Ұ���
	2. ��Ʈ�� ������ �������� ������Ʈ�� ���� �����Ұ���
	
	2-2. �ν��Ͻ� ��Ұ� ��Ʈ������Ʈ�� ���� ��ġ�� �˾Ƽ� �Ƿ��� ���� �ʿ��ұ�?
	
	3. �޾ƿ� ������ ������Ʈ�� ���� 
	���ο� �������� ������Ʈ�� ���� ���� �����Ұ���
	4. ��ǻ� ���� ������Ʈ��� �׳� ���� ������
	5. ��������� ��Ʈ ������Ʈ������ [1ȸ] �湮�� �޼ҵ�
*/
void ObjectBaseComponent::CompileComponent(const ObjectBaseComponent & OBComp)
{
	// ���� ���� ������Ʈ ���� ����
	{
		//���� ������Ʈ �ڽĵ��� �ǹ̾����� ����.
		for (ObjectBaseComponent* child : children)
		{
			child->UnlinkParentComponent();
			SafeDelete(child);
		}
		children.clear();
		children.shrink_to_fit();
	}
		
	// ��Ʈ������ �ѹ� �鷯�� �����͸� �����Ұ���
	// �ڽİ����� ��������ڷ� ��ȸ�Ұ���.
	{
		parentSocketName = OBComp.parentSocketName;
		componentName = OBComp.componentName;
		parentSocket = OBComp.parentSocket;

		baseTransform = new Transform();
		baseTransform->Local(OBComp.baseTransform->Local());
		baseTransform->Parent(OBComp.baseTransform->Parent());
	}
}

void ObjectBaseComponent::ClonningChildren(const vector<ObjectBaseComponent*>& oChildren)
{
	for (ObjectBaseComponent* child : oChildren)
	{
		ObjectBaseComponentType type = child->GetType();
		//ObjectBaseComponent* component = NULL;
		switch (type)
		{
		case ObjectBaseComponentType::ModelMesh:
			//component = new ModelMeshComponent(*dynamic_cast<ModelMeshComponent*>(child));
			LinkChildComponent( new ModelMeshComponent(*dynamic_cast<ModelMeshComponent*>(child)));
			break;
		case ObjectBaseComponentType::FigureMesh:
			LinkChildComponent( new FigureMeshComponent(*dynamic_cast<FigureMeshComponent*>(child)));
			break;
		case ObjectBaseComponentType::Camera:
			break;
		case ObjectBaseComponentType::OBB_Collision:
			LinkChildComponent( new OBB_CollisionComponent(*dynamic_cast<OBB_CollisionComponent*>(child)));
			break;
		case ObjectBaseComponentType::Sphere_Collision:
			break;
		case ObjectBaseComponentType::Capsule_Collision:
			break;
		default:
			break;
		}
		//LinkChildComponent(component);
	}
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
		
		// ���� �̸��� �޾�����, �������� �ϸ� �˾Ƽ� ì����.
		if (parentSocket < 0 && parentSocketName.compare(L"None") !=0)
		{
			Model* model = modelMesh->GetMesh();
			parentSocket = model->BoneByName(parentSocketName)->Index();
		}
		if (NULL!= animator && parentSocket >= 0)
		{
			// �ν��Ͻ���ŭ ��ġ ����.
			// �������̸� ù��° �ν��Ͻ���.
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
	{
		child->Update();
	}
}

void ObjectBaseComponent::Render()
{
	for (ObjectBaseComponent* child : children)
	{
		child->Render();
	}
}

bool ObjectBaseComponent::Property(const UINT& instance)
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

#pragma region Shader ����

void ObjectBaseComponent::SetShader(Shader * shader)
{
	this->shader = shader;
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

#pragma endregion

void ObjectBaseComponent::AddInstanceData()
{
	int index = chageTrans.size();
	chageTrans.emplace_back(false);
	if (NULL != parent)
		GetTransform(index)->Parent(parent->GetTransform(index));

	for (ObjectBaseComponent* child : children)
		child->AddInstanceData();
}

void ObjectBaseComponent::DelInstanceData(const UINT & instance)
{
	if (instance >= chageTrans.size())
		return;
	chageTrans.erase(chageTrans.begin() + instance);
	for (ObjectBaseComponent* child : children)
		child->DelInstanceData(instance);
}


#pragma region Component ����/�˾�

void ObjectBaseComponent::ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	// ������� ���ο� ���� ���� ����
	vector< ObjectBaseComponent*> heirarchyChildren = children;

	//������ ��������
	ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
	//������ ������
	ImVec2 wSize = ImGui::GetWindowSize();
	if (wSize.y < pos.y) return;
	if (wSize.x < pos.x) return;

	ImGuiTreeNodeFlags flags = heirarchyChildren.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

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
		for (auto& child : heirarchyChildren)
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
			AddCollisionComponentPopup();
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
	//TODO : ���߿� CollisionType �����ϱ�.
	if (ImGui::Button("OBB_Collision"))
	{
		OBB_CollisionComponent* addComp = new OBB_CollisionComponent();
	
		LinkChildComponent(addComp);
	}
}

#pragma endregion

#pragma region �θ��ڽ� ��ũ/��ũ
void ObjectBaseComponent::LinkParentComponent(ObjectBaseComponent * component)
{
	parent = component;

	//������Ʈ�� �ν��Ͻ� ���� ���ߴ°� ���⼭ �Ұ���.
	UINT parentInst = component->GetInstSize();
	UINT instancingCount = GetInstSize();
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

	UINT instancingCount = GetInstSize();
	// �̹� Link�Ҷ� ���� �������� �ν��Ͻ� ������ ���缭 ������ ���ָ� �ɰ�.
	for (UINT i = 0; i < instancingCount; i++)
	{
		GetTransform(i)->Parent(NULL);
	}
}

void ObjectBaseComponent::LinkChildComponent(ObjectBaseComponent * component)
{
	children.emplace_back(component);
	// ���� ����
	sort(children.begin(), children.end());
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
