#pragma once
#include "../IComponent.h"

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
	OBB_Collision,
	Sphere_Collision,
	Capsule_Collision,
};

class ObjectBaseComponent : public IComponent
{
public:
	ObjectBaseComponent();
	virtual ~ObjectBaseComponent();
	
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
	//������Ʈ �ν��Ͻ��� �ʱ� Ʈ������
	inline Transform* GetBaseTransform()		{ return baseTransform; }
	inline ObjectBaseComponentType& GetType()	{ return type; }
	inline wstring& ComponentName()				{ return componentName; }
	inline void SetEditMode(const bool& bEdit)	{ bEditMode = bEdit; }

public:
	void CompileComponent();
	void ReadyToUnlinkComp();
protected:
	Shader*		shader;
	Transform*	baseTransform;
	vector<bool> chageTrans;

	wstring	parentSocketName	= L"None";
	wstring componentName		= L"";

	int	 parentSocket = -1;
	bool bEditMode	= false;
	bool bCompiled	= false;	// ������ ���� :: 
	bool bActive	= true;		//������忡���� ���� :: ��ũ�ϸ� �⺻������ Ȱ��ȭ
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
	//�θ� ������ child ��ũ�Ҷ� �Լ� �������� ������
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