#include "Framework.h"
#include "OBB_CollisionComponent.h"



OBB_CollisionComponent::OBB_CollisionComponent()
{
	componentName = L"OBB_CollisionComp";
	type = ObjectBaseComponentType::OBB_Collision;
}


OBB_CollisionComponent::~OBB_CollisionComponent()
{
	colliders.clear();
	colliders.shrink_to_fit();
}

void OBB_CollisionComponent::Update()
{
	int loop = bEditMode ? 1 : colliders.size();
	for (int i=0;i< loop;i++)
	{
		colliders[0]->Update();
	}
	Super::Update();
}

void OBB_CollisionComponent::Render()
{
	int loop = bEditMode ? 1 : colliders.size();
	for (int i = 0; i < loop; i++)
	{
		colliders[0]->Render(lineColor);
	}
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
	colliders.emplace_back(new OBBCollider(trans));
	Super::AddInstanceData();
}

void OBB_CollisionComponent::DelInstanceData(const UINT & instance)
{
	if (instance >= colliders.size())
		return;
	colliders.erase(colliders.begin() + instance);

	Super::DelInstanceData(instance);
}

Transform * OBB_CollisionComponent::GetTransform(const UINT & instance)
{
	if (instance >= colliders.size())
		return NULL;
	return colliders[instance]->GetTransform();
}