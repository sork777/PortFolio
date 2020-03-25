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
	Collision
};

class ObjectBaseComponent : public IComponent
{
public:
	ObjectBaseComponent();
	virtual ~ObjectBaseComponent();
	
	// IComponent��(��) ���� ��ӵ�
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property();
	virtual Transform* GetTransform(const UINT& instance = 0) abstract;

	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
	virtual void SetShader(Shader* shader);
	virtual void AddInstanceData();
	virtual void DelInstanceData(const UINT& instance);

public:
	//������Ʈ �ν��Ͻ��� �ʱ� Ʈ������
	inline Transform* GetBaseTransform()		{ return baseTransform; }
	inline virtual const UINT& GetInstSize()	{ return instancingCount; }
	inline ObjectBaseComponentType& GetType()	{ return type; }
	inline wstring& ComponentName()				{ return componentName; }
	//inline vector<ObjectBaseComponent*>& GetChildrenComp() { return children; }

public:
	//�긦 ���� �������� ������ ������Ʈ �������� �� ������ Ȯ�� �����Ұ���.
	void ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp);
	void ComponentPopup();
	void AddSkeletonMeshComponentPopup();
	void AddFigureMeshComponentPopup();
	void AddCollisionComponentPopup();

protected:
	// ���� ������ ������Ʈ ��ũ.
	//�θ� ������ child ��ũ�Ҷ� �Լ� �������� ������
	void LinkParentComponent(ObjectBaseComponent* component);
	void UnlinkParentComponent();

public:
	void LinkChildComponent(ObjectBaseComponent* component);
	void UnlinkChildComponent(ObjectBaseComponent* component);

protected:
	Transform* baseTransform;
	Shader* shader;

protected:
	ObjectBaseComponentType type;

	ObjectBaseComponent*			parent;
	vector< ObjectBaseComponent*>	children;
	
	wstring	parentSocketName = L"None";
	wstring componentName	= L"";

	int	 parentSocket	= -1;
	UINT instancingCount = 0;
};