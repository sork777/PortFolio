#pragma once

#include "../BaseObject.h"

/*
	�ϴ� �� ������ ���⿡ �����ڰ� ���߿� ��������..

	���� ��ġ���� �ϰ� ���� �ִϸ��̼� �����͸� ������Ʈ ����� �����Ұ�.

*/
class Actor :public BaseObject
{
public:
	Actor(ModelMeshComponent* actorRoot);
	~Actor();

	// BaseObject��(��) ���� ��ӵ�
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;

	void AddInstanceData();
	void DelInstanceData(const UINT& instance);

	inline ModelMeshComponent* GetRootMeshData() { return root; }
private:
	ModelMeshComponent* root;

	vector<ObjectBaseComponent*> compHeirarchy;
};

