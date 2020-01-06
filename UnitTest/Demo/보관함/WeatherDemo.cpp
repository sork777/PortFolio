#include "stdafx.h"
#include "WeatherDemo.h"
#include "Environment/BillBoard.h"
#include "Environment/Sky.h"
#include "Environment/Rain.h"
#include "Environment/Snow.h"

void WeatherDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 46, -85);

	shader = new Shader(L"019_Model.fx");

	floor = new Material(shader);
	floor->DiffuseMap("Terrain/Dirt.png");
	terrain = new MeshTerrain(shader, L"HeightMap/Gray256.png");
	
	sky = new Sky(L"Environment/GrassCube1024.dds");
	
	
	
}



void WeatherDemo::Update()
{
	sky->Update();
	terrain->Update();

}

void WeatherDemo::Render()
{
	sky->Render();

	floor->Render();
	terrain->Pass(0);
	terrain->Render();
}
