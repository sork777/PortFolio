#include "Framework.h"
#include "ActorManager.h"
#include "Viewer/Orbit.h"



ActorManager::ActorManager()
{
	Initialize();
}


ActorManager::~ActorManager()
{
	Destroy();
}

void ActorManager::Initialize()
{
	selectedActor = NULL;
	bSpawningObject = false;
	bOpenActorEditor = false;
	spawnInstance = -1;
	spawnPos = Vector3(-1, -1, -1);

	float width = D3D::Width();
	float height = D3D::Height();

	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);

	Vector3 mouseVal = Vector3(Math::ToRadian(150.0f), Math::ToRadian(45.0f), 0);
	orbitCam = new Orbit(30.0f, 30.0f, 30.0f);
	
	orbitCam->CameraMove(mouseVal);
	orbitCam->Update();

	actorEditor = new ActorEditor();

}

void ActorManager::Destroy()
{
	unordered_map<UINT, Actor*>::iterator iter = actorMap.begin();
	for (; iter != actorMap.end(); iter++)
	{
		UINT objID = iter->first;
		Actor* actor = actorMap[objID];
		SafeRelease(actorSrvMap[actor]);
		SafeDelete(actor);
	}
	actorSrvMap.clear();
	actorMap.clear();

	actorList.clear();
	actorList.shrink_to_fit();
}

void ActorManager::Update()
{
	for (Actor* actor : actorList)
	{
		actor->Update();
		//�����Ϳ��� �����ϸ� Icon�� �ٲ���� �ؼ�...
		if (true == actor->IsIconReMake())
			readytoCreateList.emplace_back(actor);
	}

	if (NULL != terrain)
	{
		if (true == terrain->IsChangedHeightMap())
		{
			for (Actor* actor : actorList)
				actor->ObjectArrangementAtTerrain(terrain);
		}
	}
	ObjectSpawn();
	OpenActorEditor();
}

void ActorManager::PreRender()
{
	for (Actor* actor : actorList)
		actor->PreRender();
}

void ActorManager::Render()
{
	for (Actor* actor : actorList)
		actor->Render();

	//���Ͱ� ��� �Ǵ´�� ���⼭ �ѹ��� ��� �����Ŷ� ������
	if (readytoCreateList.size() > 0)
	{
		for (Actor* actor : readytoCreateList)
		{
			CreatActorButtonImage(actor);
			actor->ToReMakeIcon(false);
		}
		//������ ���� �� ����
		readytoCreateList.clear();
	}
}

void ActorManager::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (Actor* actor : actorList)
		actor->Tech(mesh, model, anim);
}

void ActorManager::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
	for (Actor* actor : actorList)
		actor->Pass(mesh, model, anim);
}

void ActorManager::OpenActorEditor()
{
	if (true == bSpawningObject) return;
	if (false == bOpenActorEditor) return;
	if (NULL == selectedActor) return;
	
	bOpenActorEditor = actorEditor->IsOpenedEditor();
	actorEditor->PreRender();
	actorEditor->Update();
	actorEditor->Render();
}

void ActorManager::ObjectSpawn()
{
	if (true == bOpenActorEditor) return;
	if (false == bSpawningObject) return;
	if (NULL == selectedActor) return;

	//������ �ν��Ͻ��� �����ϰ� �ִ� ���� ��ǥ�� �̵���
	selectedActor->GetTransform(spawnInstance)->Position(spawnPos);
	//������ �� �׸��� �ʱ�ȭ
	if (Mouse::Get()->Down(0))
	{
		selectedActor = NULL;
		bSpawningObject = false;
		spawnInstance = -1;
		spawnPos = Vector3(-1, -1, -1);
	}

}

Actor * ActorManager::RegistActor(Actor * actor)
{
	UINT objID = actor->GetID();
	//������ �߰�
	if (actorMap.count(objID) < 1)
	{
		actorMap[objID] = actor;
		actorList.emplace_back(actor);
		//readytoCreateList.emplace_back(actor);
		//��ư�� srv
		actorSrvMap[actor] = NULL;
		return actor;
	}
	return actorMap[objID];
}

void ActorManager::SetSpawnPosition(const Vector3 & position)
{
	if (true == bSpawningObject)
		spawnPos = position;
}

const bool& ActorManager::AddInstanceData()
{
	return selectedActor->AddInstanceData();
}

const bool& ActorManager::DelInstanceData(const UINT & instance)
{
	
	return selectedActor->DelInstanceData(instance);
}


//TODO : 0910 ������ ������ RTV�� ������ ��Ȳ �����ؾ���
void ActorManager::CreatActorButtonImage(Actor* actor)
{
	actor->ActorSyncBaseTransform();
	if (actorSrvMap.count(actor) < 1)
		RegistActor(actor);
	Actor* tempActor = new Actor(*actor);
	tempActor->SetShader(SETSHADER(L"027_Animation.fx"));
	tempActor->GetTransform()->Position(Vector3(0, 0, 0));
	tempActor->GetTransform()->Rotation(Vector3(0, 0, 0));
	// ���ͺ� ��ġ ������ ���� �κ�
	{
		Model* model = tempActor->GetRootMeshData()->GetMesh();
		model->Update();
		Vector3 autoCalVolume = model->GetMax() - model->GetMin();
		Vector3 scale;
		model->GetTransform()->Scale(&scale);
		Vector3 TargetPos = Vector3(0, autoCalVolume.y*scale.y*0.5f, 0);
		orbitCam->SetObjPos(TargetPos);
		orbitCam->Update();
	}
	//���꿡 ���� ���
	Context::Get()->SetSubCamera(orbitCam);
	renderTarget->Set(depthStencil->DSV(), Color(0.5f, 0.6f, 0.5f, 1.0f));
	{
		//��ġ �����ϰ� �ѹ� ������� ����� ������...
		tempActor->Update();
		tempActor->Update();
		tempActor->Tech(1, 1, 1);
		tempActor->Pass(0, 1, 2);
		tempActor->Render();
	}
	//TODO: 0910 �����͸� �����ϰ� �ٲ����.
	actorSrvMap[actor] = renderTarget->SRV();

	// ���ؽ�Ʈ�� ����ī�޶� ����
	Context::Get()->SetMainCamera();
}

const bool & ActorManager::ObjectIcon()
{
	/*
		���ͺ� ����
		1. ���� �̹���
		2. ���� ���� ����â
		3. ���� ������ ����â
		�ϴ���
		1. ���� ����
		2. ���ý� �ν��Ͻ� �߰� ���ɿ��ο� ���� spawn ����
		3. spawn ���ɽ� ���� ���Ϳ� ���� ���� �ν��Ͻ���ȣ �� ���� ����		
	*/
	bool bSelected = false;
	for (Actor* actor: actorList)
	{
		ImGui::Image(actorSrvMap[actor], ImVec2(80, 80));
		//ImGui�� �׷� ¦ ���߷��� ������ ������
		bool bBreak = false;
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			string btnLabel = String::ToString(actor->GetName()) + "_Add";
			bSelected = ImGui::Button(btnLabel.c_str());
			if (bSelected == true)
			{
				if (true == actor->AddInstanceData())
				{
					spawnInstance = actor->GetInstSize() - 1;
					selectedActor = actor;
					bSpawningObject = true;
					bBreak = true;
				}
				else
					bSelected = false;
			}
			btnLabel = String::ToString(actor->GetName()) + "_Show_Editor";
			bSelected = ImGui::Button(btnLabel.c_str());
			if (bSelected == true)
			{
				actorEditor->SetActor(actor);
				selectedActor = actor;
				bOpenActorEditor = true;
				bBreak = true;
			}
			ImGui::EndGroup();
		}
		if (true == bBreak)
			break;
	}
	return bSelected;
}
