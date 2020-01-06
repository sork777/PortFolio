#pragma once

enum class IconType : UINT
{
	Audio,
	Folder,
	File,
	Mesh,
	Anim,
	Model,
	HeightMap,	
	Texture,
	Custom,

	//Light,
	//Play,
	//Pause,
	//Stop,
	//Log_Info,
	//Log_Error,
	//Log_Warning,
};

struct Thumbnail
{
	Thumbnail()
		: type(IconType::Custom)
		, texture(nullptr)
		, filePath("")
	{}

	Thumbnail(const IconType& type, Texture* texture, const string& filePath)
		: type(type)
		, texture(texture)
		, filePath(filePath)
	{}

	IconType type;
	Texture* texture;
	string filePath;
};

class IconProvider
{
public:
	static IconProvider& Get()
	{
		static IconProvider instance;
		return instance;
	}

	void Initialize();

	const Thumbnail& Load
	(
		const string& filePath,
		const string& dirPath,
		const IconType& type = IconType::Custom
	);

	ID3D11ShaderResourceView* GetShaderResourceFromType(const string& dirPath, const IconType& type);
	ID3D11ShaderResourceView* GetShaderResourceFromPath(const string& filePath, const string& dirPath);

	const bool ImageButton(const string& dirPath, const IconType& type, const float& size);
	const bool ImageButton(const string& filePath, const string& dirPath, const float& size);

private:
	IconProvider() {}
	~IconProvider() {}

	const Thumbnail& GetThumbnailFromType(const IconType& type);

private:
	vector<Thumbnail> thumbnails;
};