#pragma once
#include "Framework.h"
#include "Utilities/GUID_Generator.h"
#include "Environment/Terrain/TerrainLod.h"

/*
	상속관계에 있는 클래스의 헤더파일 확인 잘해야 함 0901
	하나라도 놓치면 C2504에러남..
	오류목록에서 에러나는 부분이랑 표기되는 부분이 엇갈림
	-> 확인해야할것은 함수나 클래스가 아닌 파일명
*/


class BaseObject
{
public:
	BaseObject(){ Object_ID = GUID_Generator::Generate(); }
	virtual ~BaseObject() = default;

	const UINT& GetID() { return Object_ID; }	
	const wstring& GetName() { return Object_Name; }

	virtual void Initailize() = 0;
	virtual void Destroy() = 0;

	virtual void Update() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual const bool& ObjectProperty() = 0;

public:
	virtual void ObjectArrangementAtTerrain(TerrainLod* CurrentTerrain) abstract;

protected:	
	UINT Object_ID;
	wstring Object_Name;
};
