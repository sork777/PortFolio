#pragma once
#include "Framework.h"
#include "Utilities/GUID_Generator.h"
/*
Object
>레벨에 표기될 애들
	CubeSky
	Terrain?
	Mesh
	Model(Actor)
> ID 가지고 있을것.
>> 생성은 각자
>> 객체 추가는...?
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
