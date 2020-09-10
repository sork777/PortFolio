#pragma once
#include "Objects/BaseObject.h"
/*
	1. ����������� �긦 ���ؼ�
	2. �Ӽ��� �긦 ���ؼ�
	3. �ͷ����� �̿��� ���������� �긦 ���ؼ�
	3-2. �ͷ��� �������������� ���� ������ �����ϵ���
*/

class BillBoard;

class BillBoardObj :public BaseObject
{
public:
	BillBoardObj();
	~BillBoardObj();

	// BaseObject��(��) ���� ��ӵ�
	virtual void Initailize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual const bool& ObjectProperty() override;

	virtual void ObjectArrangementAtTerrain(TerrainLod* CurrentTerrain) override;

private:
	BillBoard* bill;
};

