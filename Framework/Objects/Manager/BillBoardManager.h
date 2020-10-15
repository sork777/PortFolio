#pragma once
#include "IObjectManager.h"
#include "Environment/BillBoard.h"
/*
	����
	1. �������� ���Ϳ� ����
	2. Ư�� ������ �ͷ��� �����Ϳ� ����
	2-1. ������ �������� �Ŵ��� 1���� ����������.
	3. ������ �̵� ���� �Լ� �ʿ�.
*/
class BillBoard;
class TerrainLod;

class BillBoardManager:public IObjectManager
{
public:
	BillBoardManager();
	~BillBoardManager();

	// IObjectManager��(��) ���� ��ӵ�
	virtual void Initialize() override;
	virtual void Destroy() override;

	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;

	void Tech(const UINT& mesh, const UINT& model, const UINT& anim);
	void Pass(const UINT& mesh, const UINT& model, const UINT& anim);

public:
	virtual void ObjectSpawn() override;
	virtual const bool & ObjectIcon() override;

private:
	virtual const bool& AddInstanceData() override;
	virtual const bool& DelInstanceData(const UINT & instance) override;

private:
	//ã�� ����(n(0)) ������
	unordered_map<UINT, BillBoard*> billMap;
	// ��ȸ ���� �ϱ� ����, ��ư�ʿ��� ���� �ʿ�
	vector< BillBoard*> billList;

	BillBoard* selectedBillBoard;

//////////////////////////////////////////////////////////////////////
// �ͷ��� ����
//////////////////////////////////////////////////////////////////////
	TerrainLod* terrain = NULL;
public:
	void SetTerrain(TerrainLod* CurrentTerrain) { terrain = CurrentTerrain; }

};

