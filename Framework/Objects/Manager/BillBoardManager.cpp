#include "Framework.h"
#include "BillBoardManager.h"
#include "Viewer/Orbit.h"



BillBoardManager::BillBoardManager()
{
	Initialize();
}


BillBoardManager::~BillBoardManager()
{
	Destroy();
}

void BillBoardManager::Initialize()
{
}

void BillBoardManager::Destroy()
{
}

void BillBoardManager::Update()
{
}

void BillBoardManager::PreRender()
{
}

void BillBoardManager::Render()
{
}

void BillBoardManager::Tech(const UINT & mesh, const UINT & model, const UINT & anim)
{
}

void BillBoardManager::Pass(const UINT & mesh, const UINT & model, const UINT & anim)
{
}

void BillBoardManager::ObjectSpawn()
{
}

const bool & BillBoardManager::ObjectIcon()
{
	// TODO: 여기에 반환 구문을 삽입합니다.
	return false;
}

const bool & BillBoardManager::AddInstanceData()
{
	// TODO: 여기에 반환 구문을 삽입합니다.
	return false;
}

const bool & BillBoardManager::DelInstanceData(const UINT & instance)
{
	// TODO: 여기에 반환 구문을 삽입합니다.
	return false;
}
