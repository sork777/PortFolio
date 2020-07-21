#include "Framework.h"
#include "OBB_CollisionComponent.h"



OBB_CollisionComponent::OBB_CollisionComponent()
{
	componentName = L"OBB_CollisionComp";
	type = ObjectBaseComponentType::OBB_Collision;
	//colliders = new OBBCollider();
}
OBB_CollisionComponent::OBB_CollisionComponent(const OBB_CollisionComponent & obbComp)
	:ObjectBaseComponent(obbComp)
{
	componentName = L"OBB_CollisionComp";
	type = ObjectBaseComponentType::OBB_Collision;
	//colliders = new OBBCollider();
	Super::ClonningChildren(obbComp.children);
}


OBB_CollisionComponent::~OBB_CollisionComponent()
{
	//for(OBBCollider* col : colliders)
	SafeDelete(colliders);
}

void OBB_CollisionComponent::Update()
{
	colliders->Update();
	Super::Update();
}

void OBB_CollisionComponent::Render()
{
	colliders->Render();

	Super::Render();
}

bool OBB_CollisionComponent::Property(const UINT& instance)
{
	if (GetInstSize() <= instance)
		return false;
	bool bChange = false;
	ImGui::ColorEdit3("LineColor", (float*)lineColor);

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
		ImGui::Separator();
		ImGui::Text("Base");
		baseTransform->Property();
	}

	bChange |= Super::Property();
	return bChange;
}


void OBB_CollisionComponent::AddInstanceData()
{
	Transform* trans = new Transform();
	trans->Local(baseTransform);

	colliders->AddInstance(trans);
		//emplace_back(new OBBCollider(trans));
	Super::AddInstanceData();
}

void OBB_CollisionComponent::DelInstanceData(const UINT & instance)
{
	if (instance >= colliders->GetSize())
		return;
	//colliders.erase(colliders.begin() + instance);
	colliders->DelInstance(instance);

	Super::DelInstanceData(instance);
}

const UINT & OBB_CollisionComponent::GetInstSize()
{
	return colliders->GetSize();
}

Transform * OBB_CollisionComponent::GetTransform(const UINT & instance)
{
	if (instance >= colliders->GetSize())
		return NULL;
	return colliders->GetTransform(instance);
}
