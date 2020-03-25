#include "Framework.h"
#include "Context.h"
#include "Viewer/Viewport.h"
#include "Viewer/Perspective.h"
#include "Viewer/Freedom.h"
#include "Viewer/Orbit.h"

Context* Context::instance = NULL;

Context * Context::Get()
{
	//assert(instance != NULL);

	return instance;
}

void Context::Create()
{
	assert(instance == NULL);

	instance = new Context();
}

void Context::Delete()
{
	SafeDelete(instance);
}

Context::Context()
{
	D3DDesc desc = D3D::GetDesc();

	//WVP에서 projection임
	perspective = new Perspective(desc.Width, desc.Height);
	viewport = new Viewport(desc.Width, desc.Height);
	//camera = new Camera(); 추상이라 안됨

	orbitCam = new Orbit(80.0f,50.0f,150.0f);		//자식이라 캐스팅
	freeCam = new Freedom();
	subCamera = NULL;

	//초기값은 프리캠
	camera = freeCam;

	lightAmbient = Color(0, 0, 0, 1);
	lightSpecular = Color(1, 1, 1, 1);
	lightDirection = Vector3(-1, -1, 1);
	lightPosition = Vector3(0, 0, 0);
}

Context::~Context()
{
	SafeDelete(freeCam);
	SafeDelete(orbitCam);
	SafeDelete(viewport);
	SafeDelete(perspective);
}

void Context::Update()
{
	camera->Update();
}

void Context::Render()
{
	viewport->RSSetViewport();

	ImGui::Begin("Context");
	{
		string str = string("Frame Rate : ") + to_string(ImGui::GetIO().Framerate);

		ImGui::Text(str.c_str());
		//Gui::Get()->RenderText(5, 5, 1, 1, 1, str);

		string Pos, Rot;

		D3DXVECTOR3 camPos;
		D3DXVECTOR3 camDir;

		if (subCamera != NULL)
		{
			Context::Get()->GetSubCamera()->Position(&camPos);
			Context::Get()->GetSubCamera()->RotationDegree(&camDir);
			Pos = "SubCam Position : ";
			Rot = "SubCam Rotation : ";
		}
		else
		{

			Context::Get()->GetCamera()->Position(&camPos);
			Context::Get()->GetCamera()->RotationDegree(&camDir);
			Pos = "Cam Position : ";
			Rot = "Cam Rotation : ";
		}

		Pos += to_string((int)camPos.x) + ", " + to_string((int)camPos.y) + ", " + to_string((int)camPos.z);
		//Gui::Get()->RenderText(5, 20, 1, 1, 1, Pos);
		ImGui::Text(Pos.c_str());

		Rot += to_string((int)camDir.x) + ", " + to_string((int)camDir.y);
		//Gui::Get()->RenderText(5, 35, 1, 1, 1, Rot);
		ImGui::Text(Rot.c_str());
		ImGui::End();
	}
}

void Context::ResizeScreen()
{
	perspective->Set(D3D::Width(), D3D::Height());
	viewport->Set(D3D::Width(), D3D::Height());
}
Matrix Context::View()
{
	Matrix view;
	if (subCamera != NULL)
	{
		subCamera->GetMatrix(&view);
		return view;
	}
	camera->GetMatrix(&view);
	return view;
}

Matrix Context::Projection()
{
	Matrix projection;
	perspective->GetMatrix(&projection);

	return projection;
}

void Context::SetFree()
{
	camera = freeCam;
}

void Context::SetObit()
{
	camera = orbitCam;
}