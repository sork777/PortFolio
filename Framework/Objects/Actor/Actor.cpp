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

	//������ ���� �ɰͰ�����...?
	//�����ؼ� �ű�� ������
	if (true == bSpawningObject)
	{
		//������ �ν��Ͻ��� �����ϰ� �ִ� ���� ��ǥ�� �̵���
		root->GetTransform(spawnInstance)->Position(spawnPos);
		//������ ��
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
	/* ���� ���� */
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
	//���� ������ ���߿� ������ ���� �ɰ� ����.
	spawnInstance = root->GetInstSize();
	root->AddInstanceData();
	bSpawningObject = true;

}

void Actor::DelInstanceData(const UINT & instance)
{
	root->DelInstanceData(instance);
}
