#include "Framework.h"
#include "MaterialPBR.h"

MaterialPBR::MaterialPBR()
{
	Initialize();
}

MaterialPBR::MaterialPBR(Shader * shader)
{
	Initialize();

	SetShader(shader);
}

void MaterialPBR::Initialize()
{
	type = BaseCompType::MaterialPBR;

	name = L"";

	albedoMap = NULL;
	heightMap = NULL;
	normalMap = NULL;
	loughnessMap = NULL;
	metalicMap = NULL;
	
	buffer = new ConstantBuffer(&colorDesc, sizeof(ColorDesc));
}

MaterialPBR::~MaterialPBR()
{
	SafeDelete(albedoMap);
	SafeDelete(heightMap);
	SafeDelete(normalMap);
	SafeDelete(loughnessMap);

	SafeDelete(buffer);
}

void MaterialPBR::SetShader(Shader * shader)
{
	this->shader = shader;

	sBuffer = shader->AsConstantBuffer("CB_MaterialPBR");

	sAlbedoMap = shader->AsSRV("AlbedoMap");
	sHeightMap = shader->AsSRV("HeightMap");
	sNormalMap = shader->AsSRV("NormalMap");
	sLoughnessMap = shader->AsSRV("LoughnessMap");
	sMetalicMap = shader->AsSRV("MetalicMap");
}

void MaterialPBR::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	if (albedoMap != NULL)
		sAlbedoMap->SetResource(albedoMap->SRV());
	else
		sAlbedoMap->SetResource(NULL);


	if (heightMap != NULL)
		sHeightMap->SetResource(heightMap->SRV());
	else
		sHeightMap->SetResource(NULL);

	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());
	else
		sNormalMap->SetResource(NULL);

	if (loughnessMap != NULL)
		sLoughnessMap->SetResource(loughnessMap->SRV());
	else
		sLoughnessMap->SetResource(NULL);

}

bool MaterialPBR::Property()
{
	int itemSize = 60;

	auto MapButton =[&](Texture* tex,string name,function<void(string,string)> func)
	{
		ImGui::PushID(name.c_str());

		Item* item;
		ID3D11ShaderResourceView* srv = NULL;
		if(tex!=NULL)
			srv = tex->SRV();
		ImGui::ImageButton(srv, ImVec2(itemSize, itemSize));

		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				func(file, dir);
			}
		}
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + itemSize*0.5f);

		ImGui::TextWrapped(name.c_str());
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - itemSize*0.5f);
		
	};

	bool bChange = false;
	{
		ImGui::PushID(this);

		ImGui::LabelText("MaterialPBRName : %s", String::ToString(name).c_str());
		ImGui::ColorEdit4("Albedo", (float*)colorDesc.Albedo);
		ImGui::ColorEdit4("F0", (float*)colorDesc.F0);
		ImGui::ColorEdit4("Emissive", (float*)colorDesc.Emissive);
		ImGui::DragFloat("Loughness", &colorDesc.Loughness,0.1f,0.0f,1.0f);
		ImGui::DragFloat("Metalic", &colorDesc.Metalic,0.1f,0.0f,1.0f);
		
		{

			MapButton(albedoMap, "AlbedoMap", bind(&MaterialPBR::LoadAlbedoMap, this, placeholders::_1, placeholders::_2));
			MapButton(heightMap, "HeightMap", bind(&MaterialPBR::LoadHeightMap, this, placeholders::_1, placeholders::_2));
			MapButton(normalMap, "NormalMap", bind(&MaterialPBR::LoadNormalMap, this, placeholders::_1, placeholders::_2));
			MapButton(loughnessMap, "LoughnessMap", bind(&MaterialPBR::LoadLoughnessMap, this, placeholders::_1, placeholders::_2));
			MapButton(metalicMap, "MetalicMap", bind(&MaterialPBR::LoadMetalicMap, this, placeholders::_1, placeholders::_2));
		}
		//ImGui::SameLine();
		ImGui::PopID();
	}

	return bChange;
}

#pragma region 색상값영역

void MaterialPBR::Albedo(Color & color)
{
	colorDesc.Albedo = color;
}

void MaterialPBR::Albedo(float r, float g, float b, float a)
{
	Albedo(Color(r, g, b, a));
}

void MaterialPBR::F0(Vector3 & color)
{
	colorDesc.F0 = color;
}

void MaterialPBR::F0(float r, float g, float b)
{
	F0(Vector3(r, g, b));
}

void MaterialPBR::Emissive(Vector3 & color)
{
	colorDesc.Emissive = color;
}

void MaterialPBR::Emissive(float r, float g, float b)
{
	Emissive(Vector3(r, g, b));
}

void MaterialPBR::Lough(float lough)
{
	colorDesc.Loughness = lough;
}

void MaterialPBR::Metalic(float  metal)
{
	colorDesc.Metalic = metal;
}


void MaterialPBR::LoadAlbedoMap(string file, string dir)
{
	LoadAlbedoMapW(String::ToWString(file), String::ToWString(dir));
}

void MaterialPBR::LoadAlbedoMapW(wstring file, wstring dir)
{
	SafeDelete(albedoMap);

	albedoMap = new Texture(file, dir);
}

void MaterialPBR::LoadHeightMap(string file, string dir)
{
	LoadHeightMapW(String::ToWString(file), String::ToWString(dir));
}

void MaterialPBR::LoadHeightMapW(wstring file, wstring dir)
{
	SafeDelete(heightMap);

	heightMap = new Texture(file, dir);
}

void MaterialPBR::LoadNormalMap(string file, string dir)
{
	LoadNormalMapW(String::ToWString(file), String::ToWString(dir));
}

void MaterialPBR::LoadNormalMapW(wstring file, wstring dir)
{
	SafeDelete(normalMap);

	normalMap = new Texture(file,dir);
}

void MaterialPBR::LoadLoughnessMap(string file, string dir)
{
	LoadLoughnessMapW(String::ToWString(file), String::ToWString(dir));

}

void MaterialPBR::LoadLoughnessMapW(wstring file, wstring dir)
{
	SafeDelete(loughnessMap);

	loughnessMap = new Texture(file, dir);
}

void MaterialPBR::LoadMetalicMap(string file, string dir)
{
	LoadMetalicMapW(String::ToWString(file), String::ToWString(dir));
}

void MaterialPBR::LoadMetalicMapW(wstring file, wstring dir)
{
	SafeDelete(metalicMap);

	metalicMap = new Texture(file, dir);
}


#pragma endregion
