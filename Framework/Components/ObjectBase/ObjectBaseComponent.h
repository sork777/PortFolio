#pragma once
#include "../IComponent.h"

/*
	Component �߰� ��ü�� �� �༮��.
		
	��Ʈ�� ������Ʈ���� ����.
	���� ����ġ�� ������Ʈ ���� �θ��� ������ ã�Ƽ� �ٴ´�.

	����-�ν��Ͻ� ��������.
	�׳� ���ο��� �ν��Ͻ�

	�������� ���� ������ ���?
*/
class Model;
class ModelAnimator;
class ModelRender;

enum class ObjectBaseComponentType
{
	ModelMesh,
	FigureMesh,
	Camera,
	OBB_Collision,
	Sphere_Collision,
	Capsule_Collision,
};

class ObjectBaseComponent : public IComponent
{
//public:
//	const UINT& GetCompID() { return compID; }
//
//protected:
	UINT compID;

public:
	//������Ʈ�� ������ �⺻ ��Ҹ� ���� �ϵ��� �ϱ� ����.
	ObjectBaseComponent();
	ObjectBaseComponent(const ObjectBaseComponent& OBComp);
	virtual ~ObjectBaseComponent();
	
	//�ϴ� �޽ð迭���� �������̵� ����.
	virtual	void CompileComponent(const ObjectBaseComponent& OBComp);
protected:
	void ClonningChildren(const vector< ObjectBaseComponent*>& oChildren);

public:
	// IComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance = 0);

	virtual void SetShader(Shader* shader);
	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
///////////////////////////////////////////////////////////////////////////////
// �ν��Ͻ� ����
public:
	virtual void AddInstanceData();
	virtual void DelInstanceData(const UINT& instance);

	virtual const UINT& GetInstSize() abstract;
	virtual Transform* GetTransform(const UINT& instance = 0) abstract;
	
public:
	void AttachSocket(const wstring& wstr) { parentSocketName = wstr; }
	const int& GetSocket()				{ return parentSocket; }
	wstring& ComponentName()			{ return componentName; }
	Transform* GetBaseTransform()		{ return baseTransform; }
	ObjectBaseComponentType& GetType()	{ return type; }

protected:
	Shader*		shader;
	//������Ʈ �ν��Ͻ��� �ʱ� Ʈ������
	Transform*	baseTransform;
	vector<bool> chageTrans;

	wstring	parentSocketName	= L"None";
	wstring componentName		= L"";

	int	 parentSocket = -1;
	
	bool bEditMode = false;
///////////////////////////////////////////////////////////////////////////////
// ������Ʈ �������� ����
public:
	//�긦 ���� �������� ������ ������Ʈ �������� �� ������ Ȯ�� �����Ұ���.
	void ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp);

private:
	void ComponentPopup();
	void AddSkeletonMeshComponentPopup();
	void AddFigureMeshComponentPopup();
	void AddCollisionComponentPopup();

private:
	// ���� ������ ������Ʈ ��ũ.
	// ������ �ÿ��� ������?
	void LinkParentComponent(ObjectBaseComponent* component);
	void UnlinkParentComponent();

public:
	void LinkChildComponent(ObjectBaseComponent* component);
	void UnlinkChildComponent(ObjectBaseComponent* component);

protected:
	ObjectBaseComponentType type;

	ObjectBaseComponent*			parent;
	vector< ObjectBaseComponent*>	children;
};