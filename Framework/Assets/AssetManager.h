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
	
	//������Ʈ ������ ���
	void RegistAsset(ContentsAsset* asset);
	//������Ʈ �Ҹ�� ����
	void UnRegistAsset(ContentsAsset* asset);
	inline vector<ContentsAsset*>& GetAllAssets() { return registedAsset; }
	//inline ContentsAsset* GetSelectedAsset()	{ return registedAsset[selectedAsset]; }
	
	const bool& ShowAssets();
	
	ContentsAsset* ViewAssets(const ContentsAssetType& type = ContentsAssetType::All);
	//ModelAsset* ViewModelAsset();
	//TextureAsset* ViewTextureAsset();

private:
	//������ ��¿�?
	vector<ContentsAsset*> registedAsset;

	//���� �з�
	//vector<ModelAsset*> registedModelAsset;
	//vector<TextureAsset*> registedTextureAsset;

	UINT selectedAsset = 0;
	//UINT selectedModelAsset = 0;
	//UINT selectedTextureAsset = 0;
};
