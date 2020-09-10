#pragma once
#include "Framework.h"

/*
	필요한 일 0902
	 Actor나 Billboard등의 여러가지 도움을 위한 클래스
	1. 객체 생성이나 인스턴스 생성을 위한 가상함수
	2. 버튼이나 에디터 이동을 위한 GUI
	3. 
*/

__interface IObjectManager
{
public:
	/*
		1. 오브젝트 이미지 보여줄 Icon
		2. Add/DelInstance용 버튼
		3. 에디터 연결용 버튼
	*/
	virtual void Initialize() abstract;
	virtual void Destroy() abstract;

	virtual void Update() abstract;
	virtual void PreRender() abstract;
	virtual void Render() abstract;

	//Add시 여기서 스폰위치 넣어줄것
	virtual void ObjectSpawn() abstract;
	virtual const bool& ObjectIcon() abstract;

	virtual const bool& AddInstanceData() abstract;
	virtual const bool& DelInstanceData(const UINT& instance) abstract;
};