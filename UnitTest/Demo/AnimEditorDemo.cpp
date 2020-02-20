#include "stdafx.h"
#include "AnimEditorDemo.h"
#include "Viewer/Freedom.h"


void AnimEditorDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(23, 0, 0);
	Context::Get()->GetCamera()->Position(0, 32, -67);
	((Freedom *)Context::Get()->GetCamera())->Speed(20, 2);

	//editor = new AnimationEditor();
	editor = new ParticleEditor();

}

void AnimEditorDemo::Destroy()
{
	SafeDelete(editor);
}

void AnimEditorDemo::Update()
{
	editor->Update();	
}

void AnimEditorDemo::PreRender()
{
}
void AnimEditorDemo::Render()
{
	editor->Render();
}
