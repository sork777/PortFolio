#pragma once
#include "../ContentsAsset.h"

/*
	모델 메시 관련
	애니메이션은 다른 에셋에.
*/
class TextureAsset : public ContentsAsset
{
public:
	TextureAsset(Texture* texture);
	TextureAsset(const wstring& path, const wstring& dir = L"../../_Textures/");
	~TextureAsset();

	// ContentsAsset을(를) 통해 상속됨
	virtual void CreateButtonImage() override;
	virtual void SaveAssetContents() override;
	virtual void LoadAssetContents() override;

	inline Texture* GetTextureAsset() { return texture; }

private:
	Texture* texture;
};