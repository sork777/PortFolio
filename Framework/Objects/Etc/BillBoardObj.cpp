#include "Framework.h"
#include "BillBoardObj.h"
#include "Environment/BillBoard.h"
//#include "Environment/Terrain/TerrainLod.h"



BillBoardObj::BillBoardObj()
{
	Initailize();
}


BillBoardObj::~BillBoardObj()
{
}

void BillBoardObj::Initailize()
{
	bill = new BillBoard();
}

void BillBoardObj::Destroy()
{
	SafeDelete(bill);
}

void BillBoardObj::Update()
{
	bill->Update();	
}

void BillBoardObj::PreRender()
{
}

void BillBoardObj::Render()
{
	bill->Render();
}

const bool& BillBoardObj::ObjectProperty()
{
	return false;
}

void BillBoardObj::ObjectArrangementAtTerrain(TerrainLod * CurrentTerrain)
{
	Vector2 scale;
	bill->GetBillBoardScale(&scale);
	int loop = bill->GetSize();
	for (int inst = 0; inst < loop; inst++)
	{
		Vector3 pos;
		bill->GetBillBoardPos(inst, &pos);
		pos.y = CurrentTerrain->GetPickedHeight(pos) + scale.y*0.5f;

		bill->SetBillBoardPos(inst, pos);
	}
}