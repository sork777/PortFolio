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
	type = CommonComponentType::Material;

	name = L"";

	diffuseMap = NULL;
	specularMap = NULL;
	normalMap = NULL;
	heightMap = NULL;
	
	buffer = new ConstantBuffer(&colorDesc, sizeof(ColorDesc));
}

Material::~Material()
{
	SafeDelete(diffuseMap);
	SafeDelete(specularMap);
	SafeDelete(normalMap);
	SafeDelete(heightMap);

	SafeDelete(buffer);
}

void Material::SetShader(Shader * shader)
{
	this->shader = shader;

	sBuffer = shader->AsConstantBuffer("CB_Material");

	sDiffuseMap = shader->AsSRV("DiffuseMap");
	sSpecularMap = shader->AsSRV("SpecularMap");
	sNormalMap = shader->AsSRV("NormalMap");
	sHeightMap = shader->AsSRV("HeightMap");
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
	if (material->HeightMap() != NULL)
		this->LoadNormalMapW(
			material->HeightMap()->GetFile(),
			material->HeightMap()->GetDir()
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

	if (heightMap != NULL)
		sHeightMap->SetResource(heightMap->SRV());
	else
		sHeightMap->SetResource(NULL);
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
		if (ImGui::ImageButton(srv, ImVec2(itemSize, itemSize)))
		{
			LoadTexture(func);
		}

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
			MapButton(specularMap, "SpecularMap", bind(&Material::LoadSpecularMap, this, placeholders::_1, placeholders::_2));
			MapButton(normalMap, "NormalMap", bind(&Material::LoadNormalMap, this, placeholders::_1, placeholders::_2));
			MapButton(heightMap, "HeightMap", bind(&Material::LoadHeightMap, this, placeholders::_1, placeholders::_2));
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

void Material::LoadDiffuseMap(const string& file, const string& dir)
{
	LoadDiffuseMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadDiffuseMapW(const wstring& file, const wstring& dir)
{
	SafeDelete(diffuseMap);

	diffuseMap = new Texture(file, dir);
}

void Material::LoadSpecularMap(const string& file, const string& dir)
{
	LoadSpecularMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadSpecularMapW(const wstring& file, const wstring& dir)
{
	SafeDelete(specularMap);

	specularMap = new Texture(file, dir);
}

void Material::LoadNormalMap(const string& file, const string& dir)
{
	LoadNormalMapW(String::ToWString(file), String::ToWString(dir));
}

void Material::LoadNormalMapW(const wstring& file, const wstring& dir)
{
	SafeDelete(normalMap);

	normalMap = new Texture(file,dir);
}
void Material::LoadHeightMap(const string & file, const string & dir)
{
	LoadHeightMapW(String::ToWString(file), String::ToWString(dir));
}
void Material::LoadHeightMapW(const wstring & file, const wstring & dir)
{
	SafeDelete(heightMap);

	heightMap = new Texture(file, dir);
}
void Material::LoadTexture(function<void(string, string)> func, const wstring& filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::ImageFilter, L"../../_Textures/", bind(&Material::LoadTexture, this,func, placeholders::_1));
	}
	else
	{
		string dir  = String::ToString( Path::GetDirectoryName(filePath));
		string file = String::ToString(Path::GetFileName(filePath));
		func(file, dir);
	}
}
#pragma endregion
