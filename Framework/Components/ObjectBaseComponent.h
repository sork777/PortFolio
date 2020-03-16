#pragma once
#include "IComponent.h"

/*
	Component �߰� ��ü�� �� �༮��.
	���� ������Ʈ���� �⺻ ������ ����
	�ν��Ͻ� �����ʹ� �ν��Ͻ� ������Ʈ���� Ȯ���Ұ�.
	�ν��Ͻ� ������Ʈ�� ������ ������Ʈ�� ���� �Ұ�.

	?? �׷��� ������Ʈ ���� �������� �ν��Ͻ� ������ ��� ����?
	-> �׳� �Լ��� �ν��Ͻ� �ѹ� �ޱ�. �������� �𸥴�...

	��Ʈ�� ������Ʈ���� ����.
	���� ����ġ�� ���?
	�𸮾��� ��� �ش� ������Ʈ���� �θ��� ������ ã�Ƽ� �ٴ´�.
*/
class Model;
class ModelAnimator;
class ModelRender;

enum class ObjectBaseComponentType
{
	ModelMesh,
	FigureMesh,
	Camera,
	Collision
};

class ObjectBaseComponent : public IComponent
{
public:
	ObjectBaseComponent();
	virtual ~ObjectBaseComponent();
	
	// IComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override {}
	virtual bool Property(const UINT& instance = 0) abstract;
	virtual Transform* GetTransform(const UINT& instance = 0) abstract;

	virtual void AddInstanceData() abstract;
	virtual void DelInstanceData(const UINT& instance) abstract;

	// ���� ������ ������Ʈ ��ũ.
protected:
	//�θ� ������ child ��ũ�Ҷ� �Լ� �������� ������
	void LinkParentComponent(ObjectBaseComponent* component, const UINT& instance = 0);
	void UnlinkParentComponent(const UINT& instance = 0);

public:
	void LinkChildComponent(ObjectBaseComponent* component, const UINT& instance = 0);
	void UnlinkChildComponent(ObjectBaseComponent* component, const UINT& instance = 0);

protected:
	inline void AttachSocket(const int& socket = -1)	{ parentSocket = socket; }
	inline ObjectBaseComponentType GetType()			{ return type; }

protected:
	ObjectBaseComponentType type;

	ObjectBaseComponent*			parent;
	vector< ObjectBaseComponent*>	children;
	
	int		parentSocket	= -1;
};