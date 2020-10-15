#include "Framework.h"
#include "OBB_CollisionComponent.h"



OBB_CollisionComponent::OBB_CollisionComponent()
{
	componentName = L"OBB_CollisionComp";
	type = ObjectBaseComponentType::OBB_Collision;
	colliders = new OBBCollider();
	//테스트용
	colliders->SetDebugMode(true);
}
OBB_CollisionComponent::OBB_CollisionComponent(const OBB_CollisionComponent & obbComp)
	:ObjectBaseComponent(obbComp)
{
	componentName = L"OBB_CollisionComp";
	type = ObjectBaseComponentType::OBB_Collision;
	colliders = new OBBCollider();
	//테스트용
	colliders->SetDebugMode(true);
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

bool OBB_CollisionComponent::Property(const int& instance)
{
	if (0 >= GetInstSize())		return false;
	if (0 >= GetInstSize() - instance)		return false;

	bool bChange = false;
	ImGui::ColorEdit3("LineColor", (float*)lineColor);

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (instance < 0)
		{
			if (GetTransform()->Property())
			{
				bChange = true;
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


void OBB_CollisionComponent::AddInstanceData()
{
	Transform* trans = new Transform();
	trans->Local(baseInitTransform);

	colliders->AddInstance(trans);
	colliders->Update();
	Super::AddInstanceData();
}

void OBB_CollisionComponent::DelInstanceData(const UINT & instance)
{
	if (instance >= colliders->GetSize())
		return;
	colliders->DelInstance(instance);
	colliders->Update();

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
