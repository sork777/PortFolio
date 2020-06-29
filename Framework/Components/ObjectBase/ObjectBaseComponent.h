#pragma once
#include "../IComponent.h"

/*
	Component 추가 객체로 들어갈 녀석들.
		
	루트는 오브젝트에서 설정.
	소켓 어태치는 컴포넌트 에서 부모의 소켓을 찾아서 붙는다.

	메인-인스턴싱 못나눴음.
	그냥 내부에서 인스턴싱

	에디터중 삭제 관련은 어떻게?
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
	//컴포넌트를 받으면 기본 요소만 복제 하도록 하기 위함.
	ObjectBaseComponent();
	ObjectBaseComponent(const ObjectBaseComponent& OBComp);
	virtual ~ObjectBaseComponent();
	
	//일단 메시계열에만 오버라이딩 하자.
	virtual	void CompileComponent(const ObjectBaseComponent& OBComp);
protected:
	void ClonningChildren(const vector< ObjectBaseComponent*>& oChildren);

public:
	// IComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override;
	virtual bool Property(const UINT& instance = 0);

	virtual void SetShader(Shader* shader);
	virtual void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	virtual void Pass(const UINT& mesh, const UINT& model, const UINT& anim);
	
///////////////////////////////////////////////////////////////////////////////
// 인스턴스 관련
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
	//컴포넌트 인스턴싱의 초기 트랜스폼
	Transform*	baseTransform;
	vector<bool> chageTrans;

	wstring	parentSocketName	= L"None";
	wstring componentName		= L"";

	int	 parentSocket = -1;
	
	bool bEditMode = false;
///////////////////////////////////////////////////////////////////////////////
// 컴포넌트 계층구조 관련
public:
	//얘를 통해 계층에서 선택한 컴포넌트 내보내고 그 정보를 확인 수정할거임.
	void ComponentHeirarchy(OUT ObjectBaseComponent** selectedComp);

private:
	void ComponentPopup();
	void AddSkeletonMeshComponentPopup();
	void AddFigureMeshComponentPopup();
	void AddCollisionComponentPopup();

private:
	// 계층 구조용 컴포넌트 링크.
	// 컴파일 시에만 쓸거임?
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