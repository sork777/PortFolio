#pragma once
#include "IObjectManager.h"
#include "Environment/BillBoard.h"
/*
	역할
	1. 여러종의 액터와 연결
	2. 특정 레벨의 터레인 데이터와 연결
	2-1. 어차피 레벨마다 매니저 1개라 괜찮을거임.
	3. 액터의 이동 관리 함수 필요.
*/
class BillBoard;
class TerrainLod;

class BillBoardManager:public IObjectManager
{
public:
	BillBoardManager();
	~BillBoardManager();

	// IObjectManager을(를) 통해 상속됨
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
	//찾기 쉽게(n(0)) 맵으로
	unordered_map<UINT, BillBoard*> billMap;
	// 순회 쉽게 하기 위함, 버튼쪽에도 쓸때 필요
	vector< BillBoard*> billList;

	BillBoard* selectedBillBoard;

//////////////////////////////////////////////////////////////////////
// 터레인 연결
//////////////////////////////////////////////////////////////////////
	TerrainLod* terrain = NULL;
public:
	void SetTerrain(TerrainLod* CurrentTerrain) { terrain = CurrentTerrain; }

};

