#include "Framework.h"
#include "ActorComponent.h"



ActorComponent::ActorComponent(Model* model)
	:mainModel(model)
{
}


ActorComponent::~ActorComponent()
{
}

void ActorComponent::Update()
{
	mainModel->Update();

	for (AttachModelData* data : attaches)
	{
		UINT attachbone = data->AttachedBoneIndex;
		for (int i = 0; i < mainModel->GetInstSize(); i++)
		{
			Matrix mat = mainModel->GetboneWorld(i, attachbone);
			data->model->GetTransform(i)->Parent(mat);
		}
		data->model->Update();
	}
}

void ActorComponent::Render()
{
	mainModel->Render();

	//Attach
	for (AttachModelData* data : attaches)
		data->model->Render();
}

void ActorComponent::Pass(UINT pass)
{
	mainModel->Pass(pass);
	for (AttachModelData* data : attaches)
		data->model->Pass(pass);
}

void ActorComponent::Tech(UINT tech)
{
	for (AttachModelData* data : attaches)
		data->model->Tech(tech);
}

void ActorComponent::AddInstance()
{
	mainModel->AddInstance();
	UINT mainSize = mainModel->GetInstSize();
	for (AttachModelData* data : attaches)
	{
		UINT attachbone = data->AttachedBoneIndex;
		Matrix mat = mainModel->GetboneWorld(mainSize-1, attachbone);

		if (data->AttachInstances.size() < mainSize)
		{
			int size = data->model->GetInstSize();
			data->AttachInstances.emplace_back(size);
			data->model->AddInstance();
			data->model->GetTransform(size)->World(
				data->model->GetTransform(data->AttachInstances[0])->World()
			);
			data->model->GetTransform(size)->Parent(mat);
		}
	}
}

void ActorComponent::DelInstance(UINT instance)
{
	if (instance >= mainModel->GetInstSize())
		return;
	mainModel->DelInstance(instance);
	for (AttachModelData* data : attaches)
	{
		UINT attachinst = data->AttachInstances[instance];
		data->AttachInstances.erase(data->AttachInstances.begin() + instance);
		data->model->DelInstance(attachinst);
	}
}

void ActorComponent::Attach(Model * model, int parentBoneIndex, UINT instanceIndex, Transform * transform)
{
	class ModelRender* render = dynamic_cast<class ModelRender*>(model);
	class ModelAnimator* animator = dynamic_cast<class ModelAnimator*>(model);

	AttachModelData* data = new AttachModelData();
	data->AttachedBoneIndex = parentBoneIndex;
	if (transform != NULL)
		model->GetTransform(instanceIndex)->Local(transform->World());
	data->AttachInstances.emplace_back(instanceIndex);
	data->model = model;
	if (render)
	{
		data->type = AttachModelType::Model_Render;
	}
	else if (animator)
	{
		data->type = AttachModelType::Model_Animator;
	}

	Matrix mat = mainModel->GetboneWorld(mainModel->GetInstSize() - 1, parentBoneIndex);

	while (mainModel->GetInstSize() > data->AttachInstances.size())
	{
		int size = data->model->GetInstSize();
		data->AttachInstances.emplace_back(size);
		data->model->AddInstance();
		data->model->GetTransform(size)->World(
			data->model->GetTransform(data->AttachInstances[0])->World()
		);
		data->model->GetTransform(size)->Parent(mat);
	}
	attaches.emplace_back(data);
}

bool ActorComponent::Property()
{
	return false;
}
