#pragma once
#include "Framework.h"
#include "Utilities/GUID_Generator.h"
#include "Environment/Terrain/TerrainLod.h"

/*
	��Ӱ��迡 �ִ� Ŭ������ ������� Ȯ�� ���ؾ� �� 0901
	�ϳ��� ��ġ�� C2504������..
	������Ͽ��� �������� �κ��̶� ǥ��Ǵ� �κ��� ������
	-> Ȯ���ؾ��Ұ��� �Լ��� Ŭ������ �ƴ� ���ϸ�
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
