#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"

#include "Convert.h"
#include "InstanceColliderDemo.h"
#include "AnimEditorDemo.h"
void Main::Initialize()
{
	//Push(new Convert());
	//Push(new InstanceColliderDemo());
	Push(new AnimEditorDemo());
}

void Main::Ready()
{

}

void Main::Destroy()
{
	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SafeDelete(exe);
	}
}

void Main::Update()
{
	for (IExecute* exe : executes)
		exe->Update();
}

void Main::PreRender()
{
	for (IExecute* exe : executes)
		exe->PreRender();
}

void Main::Render()
{
	for (IExecute* exe : executes)
		exe->Render();
}

void Main::PostRender()
{
	for (IExecute* exe : executes)
		exe->PostRender();
}

void Main::ResizeScreen()
{
	for (IExecute* exe : executes)
		exe->ResizeScreen();
}

void Main::Push(IExecute * execute)
{
	executes.push_back(execute);

	execute->Initialize();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR param, int command)
{
	D3DDesc desc;
	desc.AppName = L"D3D Game";
	desc.Instance = instance;		//프로그램 식별점, 시작주소
	desc.bFullScreen = false;
	desc.bVsync = true;
	desc.Handle = NULL;				//윈도우
	desc.Width = 1280;
	desc.Height = 720;
	desc.Background = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1);
	D3D::SetDesc(desc);


	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SafeDelete(main);

	return wParam;
}