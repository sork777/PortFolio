#pragma once
#include "Framework.h"
#include "Utilities/GUID_Generator.h"
/*
Object
>������ ǥ��� �ֵ�
	CubeSky
	Terrain?
	Mesh
	Model(Actor)
> ID ������ ������.
>> ������ ����
>> ��ü �߰���...?
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
	virtual void PostRender() = 0;

private:	
	UINT Object_ID;
	wstring Object_Name;
};
