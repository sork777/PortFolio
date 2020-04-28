#include "Actor.h"



Actor::Actor()	
	:root(NULL)
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
	if (NULL == root) return;

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
	if (NULL == root) return;
	root->Render();
}

void Actor::PostRender()
{
}

void Actor::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (NULL == root) return;
	root->Tech(mesh, model, anim);
}

void Actor::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	if (NULL == root) return;
	root->Pass(mesh, model, anim);
}

void Actor::ShowCompHeirarchy(OUT ObjectBaseComponent** selectedComp)
{
	if (NULL == root) return;
	/* 파츠 선택 */
	root->ComponentHeirarchy(selectedComp);
}

void Actor::SetSpawnPosition(const Vector3 & position)
{
	if (true == bSpawningObject)
		spawnPos = position;
}

Transform * Actor::GetTransform(const UINT & instance)
{
	return root ? root->GetTransform(instance) : NULL;
}

void Actor::AddInstanceData()
{
	if (NULL == root) return;
	//스폰 관련은 나중에 밖으로 빼도 될것 같다.
	spawnInstance = root->GetInstSize();
	root->AddInstanceData();
	bSpawningObject = true;

}

void Actor::DelInstanceData(const UINT & instance)
{
	if (NULL == root) return;
	root->DelInstanceData(instance);
}
