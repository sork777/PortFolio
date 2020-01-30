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

void Material::CloneMaterial(Material* material)
{
	this->Ambient(material->Ambient());
	this->Diffuse(material->Diffuse());
	this->Specular(material->Specular());
	this->Emissive(material->Emissive());

	if (material->DiffuseMap() != NULL)
		this->LoadDiffuseMapW(
			material->DiffuseMap()->GetFile(),
			material->DiffuseMap()->GetDir()
		);

	if (material->SpecularMap() != NULL)
		this->LoadSpecularMapW(
			material->SpecularMap()->GetFile(),
			material->SpecularMap()->GetDir()
		);

	if (material->NormalMap() != NULL)
		this->LoadNormalMapW(
			material->NormalMap()->GetFile(),
			material->NormalMap()->GetDir()
		);
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

		ImGui::LabelText("MaterialName : %s", String::ToString(name).c_str());
		ImGui::ColorEdit4("Ambient", (float*)colorDesc.Ambient);
		ImGui::ColorEdit4("Diffuse", (float*)colorDesc.Diffuse);
		ImGui::ColorEdit4("Specular", (float*)colorDesc.Specular);
		ImGui::ColorEdit4("Emissive", (float*)colorDesc.Emissive);
		
		{

			MapButton(diffuseMap, "DiffuseMap", bind(&Material::LoadDiffuseMap, this, placeholders::_1, placeholders::_2));
			MapButton(specularMap, "SpeculatMap", bind(&Material::LoadSpecularMap, this, placeholders::_1, placeholders::_2));
			MapButton(normalMap, "NormalMap", bind(&Material::LoadNormalMap, this, placeholders::_1, placeholders::_2));
		}
		//ImGui::SameLine();
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

void Material::LoadDiffuseMap(string file, string dir)
{
	LoadDiffuseMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadDiffuseMapW(wstring file, wstring dir)
{
	SafeDelete(diffuseMap);

	diffuseMap = new Texture(file, dir);
}

void Material::LoadSpecularMap(string file, string dir)
{
	LoadSpecularMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadSpecularMapW(wstring file, wstring dir)
{
	SafeDelete(specularMap);

	specularMap = new Texture(file, dir);
}

void Material::LoadNormalMap(string file, string dir)
{
	LoadNormalMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadNormalMapW(wstring file, wstring dir)
{
	SafeDelete(normalMap);

	normalMap = new Texture(file,dir);
}
#pragma endregion
