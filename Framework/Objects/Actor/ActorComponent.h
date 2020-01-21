#pragma once
/*
모델 애니메이터 갖음
어태치 목록 있음
인풋 컴포넌트 생성 예정

*/

enum AttachModelType
{
	Model_None,
	Model_Render,
	Model_Animator
};

struct AttachModelData
{
	AttachModelType type = AttachModelType::Model_None;
	UINT AttachedBoneIndex = -1;
	vector<UINT> AttachInstances;
	class Model* model = NULL;
};

class ActorComponent
{
public:
	ActorComponent(Model* model);
	virtual ~ActorComponent();

	void Update();
	void Render();

	void Pass(UINT pass);
	void Tech(UINT tech);

	void AddInstance();
	void DelInstance(UINT instance);

	void Attach(Model* model, int parentBoneIndex, UINT instanceIndex, Transform* transform = NULL);
	bool Property();

public:
	Model* GetModel() { return mainModel; }
private:
	Model* mainModel;
	vector< AttachModelData * > attaches;

};

