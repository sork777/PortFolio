#include "Framework.h"
#include "ObjectBaseComponent.h"

ObjectBaseComponent::ObjectBaseComponent()
	:shader(NULL), parent(NULL)
{
	baseTransform = new Transform();
	compID = GUID_Generator::Generate();

	std::cout << "현재 컴포넌트 주소 : " << this << endl;
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
	//베이스는 부모에 트랜스폼을 얹지 않는다.
	baseTransform->Parent(OBComp.baseTransform->Parent());

	std::cout << "원본 컴포넌트 주소 : " << &OBComp << endl;
	std::cout << "현재 컴포넌트 주소 : " << this << endl;

}

ObjectBaseComponent::~ObjectBaseComponent()
{
	SafeDelete(baseTransform);
	
	if (NULL == parent)
	{
		// Root 컴포넌트일때. Root가 삭제되면서 전부 삭제될때.
		for (ObjectBaseComponent* child : children)
		{
			//순차적으로 링크 끊고 해제하기
			child->UnlinkParentComponent();
			SafeDelete(child);
		}		
	}
	else
	{
		// 삭제시 상위 컴포넌트에 차일드 넘기기
		for (ObjectBaseComponent* child : children)
			parent->LinkChildComponent(child);
	}
	children.clear();
	children.shrink_to_fit();
	//쉐이더는 밖에서 오는거라 삭제는 패스
//	
}


/*
	1. 에딧용 액터에서 컴포넌트를 받아서 복사할거임
	2. 루트를 제외한 하위계층 컴포넌트는 전부 삭제할거임
	
	2-2. 인스턴싱 요소가 루트컴포넌트에 의해 위치가 알아서 되려면 뭐가 필요할까?
	
	3. 받아온 에디터 컴포넌트를 통해 
	새로운 하위계층 컴포넌트로 전부 복사 생성할거임
	4. 사실상 같은 컴포넌트였어도 그냥 새로 쓸거임
	5. 결론적으로 루트 컴포넌트에서만 [1회] 방문할 메소드
*/
void ObjectBaseComponent::CompileComponent(const ObjectBaseComponent & OBComp)
{
	// 기존 하위 컴포넌트 삭제 과정
	{
		//기존 컴포넌트 자식들은 의미없으니 삭제.
		for (ObjectBaseComponent* child : children)
		{
			child->UnlinkParentComponent();
			SafeDelete(child);
		}
		children.clear();
		children.shrink_to_fit();
	}
		
	// 루트에서만 한번 들러서 데이터를 복사할거임
	// 자식계층은 복사생성자로 순회할거임.
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
	// 모델 메시 아니면 소켓이 필요 없음.
	// 매번 위치 바꿔줄 필요도 없음.

	else if (parent->GetType() == ObjectBaseComponentType::ModelMesh)
	{

		//부모를 다이나믹 형변환해서 컴포넌트 추출
		ModelMeshComponent* modelMesh = dynamic_cast<ModelMeshComponent*>(parent);
		// 모델과 애니메이터 추출
		ModelAnimator* animator = modelMesh->GetAnimation();
		
		// 소켓 이름만 받았을시, 설정으로 하면 알아서 챙겨짐.
		if (parentSocket < 0 && parentSocketName.compare(L"None") !=0)
		{
			Model* model = modelMesh->GetMesh();
			parentSocket = model->BoneByName(parentSocketName)->Index();
		}
		if (NULL!= animator && parentSocket >= 0)
		{
			// 인스턴스만큼 위치 업뎃.
			// 에딧중이면 첫번째 인스턴스만.
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

	//루트만 업데이트 호출하면 자식 자동 호출하게
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

#pragma region Shader 관련

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


#pragma region Component 계층/팝업

void ObjectBaseComponent::ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	// 에딧모드 여부에 따라 계층 선택
	vector< ObjectBaseComponent*> heirarchyChildren = children;

	//아이템 시작지점
	ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
	//윈도우 사이즈
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
	//TODO : 에셋 매니져에 등록된 FigureMesh에서 컴포넌트 생성해서 받기
}

void ObjectBaseComponent::AddCollisionComponentPopup()
{
	//TODO : 나중에 CollisionType 선택하기.
	if (ImGui::Button("OBB_Collision"))
	{
		OBB_CollisionComponent* addComp = new OBB_CollisionComponent();
	
		LinkChildComponent(addComp);
	}
}

#pragma endregion

#pragma region 부모자식 링크/언링크
void ObjectBaseComponent::LinkParentComponent(ObjectBaseComponent * component)
{
	parent = component;

	//컴포넌트의 인스턴싱 갯수 맞추는건 여기서 할거임.
	UINT parentInst = component->GetInstSize();
	UINT instancingCount = GetInstSize();
	UINT maxLoop = parentInst > instancingCount ? parentInst : instancingCount;
	for (UINT i = 0; i < maxLoop; i++)
	{
		//부모 컴포넌트의 인스턴싱 갯수가 더 적음
		if (parentInst - 1 < i)
		{
			component->AddInstanceData();
		}
		//자식 컴포넌트의 인스턴싱 갯수가 더 적음
		else if (instancingCount < i + 1)
		{
			AddInstanceData();
		}

		//위치 조정
		GetTransform(i)->Parent(component->GetTransform(i));
	}
	
}

void ObjectBaseComponent::UnlinkParentComponent()
{
	parent = NULL;

	UINT instancingCount = GetInstSize();
	// 이미 Link할때 갯수 맞췄으니 인스턴싱 데이터 맞춰서 삭제만 해주면 될것.
	for (UINT i = 0; i < instancingCount; i++)
	{
		GetTransform(i)->Parent(NULL);
	}
}

void ObjectBaseComponent::LinkChildComponent(ObjectBaseComponent * component)
{
	children.emplace_back(component);
	// 벡터 정렬
	sort(children.begin(), children.end());
	//자식의 쉐이더도 자신의 쉐이더와 통일
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
