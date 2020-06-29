#include "Actor.h"



Actor::Actor()	
	:root(NULL)
{
	Initailize();
}

Actor::Actor(Model & model)
{
	root = new ModelMeshComponent(&model);
	Initailize();
}

Actor::Actor(const Actor& actor)
{
	bEditMode = true;
	root = new ModelMeshComponent(*(actor.root));
	root->AddInstanceData();

	Initailize();
}


Actor::~Actor()
{
	Destroy();
}

void Actor::Initailize()
{
	/*std::cout << (!bEditMode ? "���ξ���" : "������ ����") << "����" << endl;
	std::cout << "��Ʈ �޽� �ּ� : " << root << endl;
*/
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
		if (true == bEditMode) return;
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
	if (true == bEditMode) return;
	if (true == bSpawningObject)
		spawnPos = position;
}

Transform * Actor::GetTransform(const UINT & instance)
{
	if (true == bEditMode) return root->GetTransform();

	return root ? root->GetTransform(instance) : NULL;
}

void Actor::AddInstanceData()
{
	//�����϶� �Ⱦ�
	if (true == bEditMode) return;
	if (NULL == root) return;
	//���� ������ ���߿� ������ ���� �ɰ� ����.
	spawnInstance = root->GetInstSize();
	root->AddInstanceData();
	bSpawningObject = true;

}

void Actor::DelInstanceData(const UINT & instance)
{
	if (true == bEditMode) return;
	if (NULL == root) return;
	root->DelInstanceData(instance);
}

void Actor::ActorCompile(const Actor& editActor)
{
	root->CompileComponent(*editActor.root);
}

void Actor::SetRootComponent(ModelMeshComponent * actorRoot)
{
	//���� ����� ���ʹ� �̹� ��Ʈ�� �������ִ¾���.
	if (true == bEditMode) return;
	root = actorRoot;
}