#pragma once
#include "../IComponent.h"

/*
	Component 추가 객체로 들어갈 녀석들.
	메인 오브젝트에서 기본 설정값 들어가고
	인스턴스 데이터는 인스턴싱 오브젝트에서 확인할것.
	인스턴싱 오브젝트는 메인의 컴포넌트를 공유 할것.

	?? 그러나 컴포넌트 계층 구조에서 인스턴싱 구분을 어떻게 하지?
	-> 그냥 함수에 인스턴스 넘버 받기. 나머지는 모른다...

	루트는 오브젝트에서 설정.
	소켓 어태치는 어디서?
	언리얼의 경우 해당 컴포넌트에서 부모의 소켓을 찾아서 붙는다.
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
	//컴포넌트 인스턴싱의 초기 트랜스폼
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
	bool bCompiled	= false;	// 씬에서 적용 :: 
	bool bActive	= true;		//에딧모드에서만 적용 :: 링크하면 기본적으로 활성화
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
	//부모 설정은 child 링크할때 함수 내에서만 쓸거임
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