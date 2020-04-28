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
	/* ���� ���� */
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
	//���� ������ ���߿� ������ ���� �ɰ� ����.
	spawnInstance = root->GetInstSize();
	root->AddInstanceData();
	bSpawningObject = true;

}

void Actor::DelInstanceData(const UINT & instance)
{
	if (NULL == root) return;
	root->DelInstanceData(instance);
}
