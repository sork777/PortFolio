#include "Actor.h"



Actor::Actor(ModelMeshComponent* actorRoot)
	:root(actorRoot)
{
	Initailize();
}


Actor::~Actor()
{
	Destroy();
}

void Actor::Initailize()
{
	bSpawningObject = false;
	spawnInstance = -1;
	spawnPos = Vector3(-1, -1, -1);
}

void Actor::Destroy()
{
	SafeDelete(root);
}

void Actor::Update()
{
	root->Update();

	//밖으로 빼도 될것같은데...?
	//생성해서 옮기는 도중임
	if (true == bSpawningObject)
	{
		//스폰한 인스턴스를 갱신하고 있는 스폰 좌표로 이동중
		root->GetTransform(spawnInstance)->Position(spawnPos);
		//놓으면 끝
		if(Mouse::Get()->Down(0))
			bSpawningObject = false;
	}
}

void Actor::PreRender()
{
}

void Actor::Render()
{
	root->Render();
}

void Actor::PostRender()
{
}

void Actor::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	root->Tech(mesh, model, anim);
}

void Actor::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	root->Pass(mesh, model, anim);
}

void Actor::ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	bool bDocking = true;
	/* 파츠 선택 */
	ImGui::Begin("ComponentHierarchy", &bDocking);
	{
		root->ComponentHeirarchy(selectedComp);
	}
	ImGui::End();
}

void Actor::SetSpawnPosition(const Vector3 & position)
{
	if (true == bSpawningObject)
		spawnPos = position;
}

Transform * Actor::GetTransform(const UINT & instance)
{
	return root->GetTransform(instance);
}

void Actor::AddInstanceData()
{
	//스폰 관련은 나중에 밖으로 빼도 될것 같다.
	spawnInstance = root->GetInstSize();
	root->AddInstanceData();
	bSpawningObject = true;

}

void Actor::DelInstanceData(const UINT & instance)
{
	root->DelInstanceData(instance);
}
