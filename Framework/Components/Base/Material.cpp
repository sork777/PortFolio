#include "Framework.h"
#include "Material.h"

Material::Material()
{
	Initialize();
}

Material::Material(Shader * shader)
{
	Initialize();

	SetShader(shader);
}

void Material::Initialize()
{
	type = BaseCompType::Material;

	name = L"";

	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;

	buffer = new ConstantBuffer(&colorDesc, sizeof(ColorDesc));
}

Material::~Material()
{
	SafeDelete(diffuseMap);
	SafeDelete(specularMap);
	SafeDelete(normalMap);

	SafeDelete(buffer);
}

void Material::SetShader(Shader * shader)
{
	this->shader = shader;

	sBuffer = shader->AsConstantBuffer("CB_Material");

	sDiffuseMap = shader->AsSRV("DiffuseMap");
	sSpecularMap = shader->AsSRV("SpecularMap");
	sNormalMap = shader->AsSRV("NormalMap");
}

void Material::Render()
{
	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	if (diffuseMap != NULL)
		sDiffuseMap->SetResource(diffuseMap->SRV());
	else
		sDiffuseMap->SetResource(NULL);


	if (specularMap != NULL)
		sSpecularMap->SetResource(specularMap->SRV());
	else
		sSpecularMap->SetResource(NULL);

	if (normalMap != NULL)
		sNormalMap->SetResource(normalMap->SRV());
	else
		sNormalMap->SetResource(NULL);
}

bool Material::Property()
{
	bool bChange = false;
	{
		ImGui::PushID(this);

		ImGui::LabelText("MaterialName : %s", String::ToString(name).c_str());
		ImGui::ColorEdit4("Ambient", (float*)colorDesc.Ambient);
		ImGui::ColorEdit4("Diffuse", (float*)colorDesc.Diffuse);
		ImGui::ColorEdit4("Specular", (float*)colorDesc.Specular);
		ImGui::ColorEdit4("Emissive", (float*)colorDesc.Emissive);

		ImGui::ImageButton(diffuseMap->SRV(), ImVec2(50, 50));
		Item* item;
		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				DiffuseMap(file, dir);
			}
		}

		ImGui::SameLine();

		ImGui::ImageButton(specularMap->SRV(), ImVec2(50, 50));
		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				SpecularMap(file, dir);
			}
		}
		ImGui::SameLine();
		ImGui::ImageButton(normalMap->SRV(), ImVec2(50, 50));
		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				NormalMap(file, dir);
			}
		}
		ImGui::PopID();
	}

	return bChange;
}

#pragma region 색상값영역

void Material::Ambient(Color & color)
{
	colorDesc.Ambient = color;
}

void Material::Ambient(float r, float g, float b, float a)
{
	Ambient(Color(r, g, b, a));
}

void Material::Diffuse(Color & color)
{
	colorDesc.Diffuse = color;
}

void Material::Diffuse(float r, float g, float b, float a)
{
	Diffuse(Color(r, g, b, a));
}

void Material::Specular(Color & color)
{
	colorDesc.Specular = color;
}

void Material::Specular(float r, float g, float b, float a)
{
	Specular(Color(r, g, b, a));
}

void Material::Emissive(Color & color)
{
	colorDesc.Emissive = color;
}

void Material::Emissive(float r, float g, float b, float a)
{
	Emissive(Color(r, g, b, a));
}

void Material::DiffuseMap(string file, string dir)
{
	DiffuseMap(String::ToWString(file), String::ToWString(dir));
}

void Material::DiffuseMap(wstring file, wstring dir)
{
	SafeDelete(diffuseMap);

	diffuseMap = new Texture(file, dir);
}

void Material::SpecularMap(string file, string dir)
{
	SpecularMap(String::ToWString(file), String::ToWString(dir));
}

void Material::SpecularMap(wstring file, wstring dir)
{
	SafeDelete(specularMap);

	specularMap = new Texture(file, dir);
}

void Material::NormalMap(string file, string dir)
{
	NormalMap(String::ToWString(file), String::ToWString(dir));
}

void Material::NormalMap(wstring file, wstring dir)
{
	SafeDelete(normalMap);

	normalMap = new Texture(file,dir);
}


#pragma endregion
