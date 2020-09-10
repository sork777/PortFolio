#pragma once
#include "Framework.h"

/*
	�ʿ��� �� 0902
	 Actor�� Billboard���� �������� ������ ���� Ŭ����
	1. ��ü �����̳� �ν��Ͻ� ������ ���� �����Լ�
	2. ��ư�̳� ������ �̵��� ���� GUI
	3. 
*/

__interface IObjectManager
{
public:
	/*
		1. ������Ʈ �̹��� ������ Icon
		2. Add/DelInstance�� ��ư
		3. ������ ����� ��ư
	*/
	virtual void Initialize() abstract;
	virtual void Destroy() abstract;

	virtual void Update() abstract;
	virtual void PreRender() abstract;
	virtual void Render() abstract;

	//Add�� ���⼭ ������ġ �־��ٰ�
	virtual void ObjectSpawn() abstract;
	virtual const bool& ObjectIcon() abstract;

	virtual const bool& AddInstanceData() abstract;
	virtual const bool& DelInstanceData(const UINT& instance) abstract;
};