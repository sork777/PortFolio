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
	BaseObject(){ id = GUID_Generator::Generate(); }
	virtual ~BaseObject() = default;

	const UINT& GetID() { return id; }
	
	virtual void Initailize() = 0;
	virtual void Destroy() = 0;

	virtual void Update() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual void PostRender() = 0;

private:	
	UINT id;
};
