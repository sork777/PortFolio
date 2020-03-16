#pragma once
#include "IComponent.h"

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
	Collision
};

class ObjectBaseComponent : public IComponent
{
public:
	ObjectBaseComponent();
	virtual ~ObjectBaseComponent();
	
	// IComponent을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render() override {}
	virtual bool Property(const UINT& instance = 0) abstract;
	virtual Transform* GetTransform(const UINT& instance = 0) abstract;

	virtual void AddInstanceData() abstract;
	virtual void DelInstanceData(const UINT& instance) abstract;

	// 계층 구조용 컴포넌트 링크.
protected:
	//부모 설정은 child 링크할때 함수 내에서만 쓸거임
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