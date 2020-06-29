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
	/*std::cout << (!bEditMode ? "메인액터" : "에디터 액터") << "생성" << endl;
	std::cout << "루트 메시 주소 : " << root << endl;
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


	//밖으로 빼도 될것같은데...?
	//생성해서 옮기는 도중임
	if (true == bSpawningObject)
	{
		if (true == bEditMode) return;
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
	//에딧일때 안씀
	if (true == bEditMode) return;
	if (NULL == root) return;
	//스폰 관련은 나중에 밖으로 빼도 될것 같다.
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
	//에딧 모드인 액터는 이미 루트가 정해져있는애임.
	if (true == bEditMode) return;
	root = actorRoot;
}