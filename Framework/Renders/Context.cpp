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

	pointLightCount = 0;
	spotLightCount = 0;
	capsuleLightCount = 0;
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
	if(Keyboard::Get()->Down('1'))
		camera = freeCam;
	else if (Keyboard::Get()->Down('2'))
		camera = orbitCam;

	camera->Update();
}

void Context::Render()
{
	viewport->RSSetViewport();

	string str = string("Frame Rate : ") + to_string(ImGui::GetIO().Framerate);
	Gui::Get()->RenderText(5, 5, 1, 1, 1, str);

	D3DXVECTOR3 camPos;
	Context::Get()->GetCamera()->Position(&camPos);

	D3DXVECTOR3 camDir;
	Context::Get()->GetCamera()->RotationDegree(&camDir);

	str = "Cam Position : ";
	str += to_string((int)camPos.x) + ", " + to_string((int)camPos.y) + ", " + to_string((int)camPos.z);
	Gui::Get()->RenderText(5, 20, 1, 1, 1, str);

	str = "Cam Rotation : ";
	str += to_string((int)camDir.x) + ", " + to_string((int)camDir.y);
	Gui::Get()->RenderText(5, 35, 1, 1, 1, str);

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

UINT Context::PointLights(OUT PointLight * lights)
{
	memcpy(lights, pointLights, sizeof(PointLight) * pointLightCount);

	return pointLightCount;
}

UINT Context::PointLightsCount()
{
	return pointLightCount;
}

void Context::AddPointLight(PointLight & light)
{
	pointLights[pointLightCount] = light;
	pointLightCount++;
}

PointLight & Context::GetPointLight(UINT index)
{
	return pointLights[index];
}

UINT Context::SpotLights(OUT SpotLight * lights)
{
	memcpy(lights, spotLights, sizeof(SpotLight) * spotLightCount);

	return spotLightCount;
}

UINT Context::SpotLightsCount()
{
	return spotLightCount;
}

void Context::AddSpotLight(SpotLight & light)
{
	spotLights[spotLightCount] = light;
	spotLightCount++;
}

SpotLight & Context::GetSpotLight(UINT index)
{
	return spotLights[index];
}

UINT Context::CapsuleLights(OUT CapsuleLight * lights)
{
	memcpy(lights, capsuleLights, sizeof(CapsuleLight) * capsuleLightCount);

	return capsuleLightCount;
}

UINT Context::CapsuleLightsCount()
{
	return capsuleLightCount;
}

void Context::AddCapsuleLight(CapsuleLight & light)
{
	capsuleLights[capsuleLightCount] = light;
	capsuleLightCount++;
}

CapsuleLight & Context::GetCapsuleLight(UINT index)
{
	return capsuleLights[index];
}
