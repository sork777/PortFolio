#include "Framework.h"
#include "ObjectPicker.h"



ObjectPicker::ObjectPicker()
{
	shader = SETSHADER(L"PF_ObjectPicker.fx");
	perframe = new PerFrame(shader);

	float width = D3D::Width();
	float height = D3D::Height();


	screen = new Render2D();
	screen->GetTransform()->Position(width*0.5f, height*0.5f, 0);
	screen->GetTransform()->Scale(width, height, 1);

	targetColorID = new RenderTarget((UINT)width, (UINT)height);
	targetInst = new RenderTarget((UINT)width, (UINT)height);

	depthStencil = new DepthStencil((UINT)width, (UINT)height, true);

	viewport = new Viewport((UINT)width, (UINT)height);

	///////////////////////////////////////////////////////
	/* 텍스쳐 보내기*/
	shader->AsSRV("ColorIDTexture")->SetResource(targetColorID->SRV());
	shader->AsSRV("InstTexture")->SetResource(targetInst->SRV());
}


ObjectPicker::~ObjectPicker()
{
}

void ObjectPicker::SetObject()
{
}

void ObjectPicker::Render()
{
}
