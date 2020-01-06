#include "Framework.h"
#include "Projector.h"
#include "Viewer/Fixity.h"

Projector::Projector(Shader * shader, wstring textureFile, bool bOrtho, UINT width, UINT height)
	: shader(shader), width(width), height(height),bOrtho(bOrtho)
{
	camera = new Fixity();
	camera->RotationDegree(90, 0, 0);
	camera->Position(0, 10, -10);

	if(bOrtho)
		projection = new Orthographic((float)width, (float)height);
	else
		projection = new Perspective((float)width, (float)height, 0.1f, 100.0f, Math::PI * 0.25f);
	textures.emplace_back(new Texture(textureFile));
	buffer = new ConstantBuffer(&desc, sizeof(Desc));


	sMap = shader->AsSRV("ProjectorMap");
	sMap->SetResource(textures[0]->SRV());

	sBuffer = shader->AsConstantBuffer("CB_Projector");
}

Projector::~Projector()
{
	SafeDelete(camera);
	SafeDelete(projection);

	textures.clear();
	textures.shrink_to_fit();

	SafeDelete(buffer);
}

void Projector::Update()
{
	Vector3 position;
	camera->Position(&position);

	ImGui::SliderFloat3("Position", (float *)&position, -80, 80);
	camera->Position(position);


	ImGui::InputInt("Width", (int *)&width);
	width = width < 1 ? 1 : width;

	ImGui::InputInt("Height", (int *)&height);
	height = height < 1 ? 1 : height;

	if (bOrtho)
		((Orthographic *)projection)->Set((float)width, (float)height);
	else
		((Perspective *)projection)->Set((float)width, (float)height, 0.1f, 100.0f, Math::PI * 0.25f);

	ImGui::ColorEdit4("Color", (float *)&desc.Color);


	camera->GetMatrix(&desc.View);
	projection->GetMatrix(&desc.Projection);
	/* 변경 될때만 하는게 좋음 */
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}

void Projector::AddTexture(wstring textureFile)
{
	textures.emplace_back(new Texture(textureFile));
}

void Projector::SelectTex(int index)
{
	/* 인덱스가 음수면 안그림 */
	if (index < 0)
		desc.draw = -1;
	else
		desc.draw = 1;

	if (index >= textures.size())
		return;
	sMap->SetResource(textures[index]->SRV());
}
