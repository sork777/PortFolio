#include "Framework.h"
#include "IconProvider.h"

static Thumbnail noThumbnail;

void IconProvider::Initialize()
{
	//Load("Icon/Audio.png","../", IconType::Audio);
	//Load("Icon/CameraGizmo.png", "../", IconType::Camera);
	//Load("Icon/LightGizmo.png", "../", IconType::Light);
	Load("Icon/Icon_Folder.png", "../", IconType::Folder);
	Load("Icon/Icon_File.png", "../", IconType::File);
	//Load("Icon/playButton.png", "../", IconType::Play);
	//Load("Icon/pauseButton.png", "../", IconType::Pause);
	//Load("Icon/stopButton.png", "../", IconType::Stop);
	//Load("Icon/log_info.png", "../", IconType::Log_Info);
	//Load("Icon/log_warning.png", "../", IconType::Log_Warning);
	//Load("Icon/log_error.png", "../", IconType::Log_Error);
}

const Thumbnail & IconProvider::Load(const string & filePath, const string& dirPath, const IconType & type)
{
	if (Path::IsSupportTextureFile(filePath))
	{
		Texture* texture = new Texture(String::ToWString( filePath), String::ToWString(dirPath));

		thumbnails.emplace_back(type, texture, filePath);
		return thumbnails.back();
	}

	return GetThumbnailFromType(IconType::File);
}

ID3D11ShaderResourceView * IconProvider::GetShaderResourceFromType(const string& dirPath, const IconType & type)
{
	return Load("", dirPath,type).texture->SRV();
}

ID3D11ShaderResourceView * IconProvider::GetShaderResourceFromPath(const string & filePath, const string& dirPath)
{
	return Load(filePath, dirPath).texture->SRV();
}

const bool IconProvider::ImageButton(const string& dirPath, const IconType & type, const float & size)
{
	bool bPressed = ImGui::ImageButton
	(
		GetShaderResourceFromType(dirPath,type),
		ImVec2(size, size)
	);

	return bPressed;
}

const bool IconProvider::ImageButton(const string & filePath, const string& dirPath, const float & size)
{
	bool bPressed = ImGui::ImageButton
	(
		GetShaderResourceFromPath(filePath, dirPath),
		ImVec2(size, size)
	);

	return bPressed;
}

const Thumbnail & IconProvider::GetThumbnailFromType(const IconType & type)
{
	for (auto& thumbnail : thumbnails)
	{
		if (thumbnail.type == type)
			return thumbnail;
	}

	return noThumbnail;
}
