#pragma once
#include "Objects/BaseObject.h"
/*
	1. 빌보드생성은 얘를 통해서
	2. 속성도 얘를 통해서
	3. 터레인을 이용한 높이조절도 얘를 통해서
	3-2. 터레인 높이조절적용을 할지 말지도 선택하도록
*/

class BillBoard;

class BillBoardObj :public BaseObject
{
public:
	BillBoardObj();
	~BillBoardObj();

	// BaseObject을(를) 통해 상속됨
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

