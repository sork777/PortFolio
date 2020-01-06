#include "stdafx.h"
#include "TerrainDemo.h"
#include "Viewer/Projector.h"
#include "Environment/Terrain.h"

void TerrainDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"036_Projector.fx");
	projector = new Projector(shader, L"Rect_Brush.png",true);
	projector->AddTexture(L"Circle_Brush.png");

	terrain = new Terrain(shader, L"HeightMap/Gray256.png");
	terrain->BaseMap(L"Terrain/Dirt.png");
	type = -1;
	projector->SelectTex(type);
}

void TerrainDemo::Update()
{
	terrain->Update();
	int type0=terrain->GetBrushType()-1;

	/* 값이 변경 될때만 함수 호출 */
	if (type != type0)
	{
		type = type0;
		projector->SelectTex(type);
	}

	/* 마우스의 터레인 피킹 좌표 */
	Vector3 picked = terrain->GetPickedPosition();
	/* 반지름 기준이라 2배 + 여유분? */
	float size = terrain->GetBrushRange()*2+4;
	if(Mouse::Get()->Press(1)==false)
		projector->GetCamera()->Position(picked);
	projector->SetWH(size, size);
	projector->Update();
}

void TerrainDemo::Render()
{
	terrain->Pass(3);
	terrain->Render();
}
