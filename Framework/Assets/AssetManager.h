#pragma once

class AssetManager
{
public:
	static AssetManager* Get();
	static void Create();
	static void Delete();

private:
	static AssetManager* instance;

public:
	AssetManager();
	~AssetManager();
	
	//컴포넌트 생성시 등록
	void RegistAsset(ContentsAsset* asset);
	//컴포넌트 소멸시 해제
	void UnRegistAsset(ContentsAsset* asset);
	inline vector<ContentsAsset*>& GetAllAssets() { return registedAsset; }
	//inline ContentsAsset* GetSelectedAsset()	{ return registedAsset[selectedAsset]; }
	
	const bool& ShowAssets();
	
	ContentsAsset* ViewAssets(const ContentsAssetType& type = ContentsAssetType::All);
	//ModelAsset* ViewModelAsset();
	//TextureAsset* ViewTextureAsset();

private:
	//브라우저 출력용?
	vector<ContentsAsset*> registedAsset;

	//에셋 분류
	//vector<ModelAsset*> registedModelAsset;
	//vector<TextureAsset*> registedTextureAsset;

	UINT selectedAsset = 0;
	//UINT selectedModelAsset = 0;
	//UINT selectedTextureAsset = 0;
};
