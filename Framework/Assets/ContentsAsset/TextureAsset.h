#pragma once
#include "../ContentsAsset.h"

/*
	�� �޽� ����
	�ִϸ��̼��� �ٸ� ���¿�.
*/
class TextureAsset : public ContentsAsset
{
public:
	TextureAsset(Texture* texture);
	TextureAsset(const wstring& path, const wstring& dir = L"../../_Textures/");
	~TextureAsset();

	// ContentsAsset��(��) ���� ��ӵ�
	virtual void CreateButtonImage() override;
	virtual void SaveAssetContents() override;
	virtual void LoadAssetContents() override;

	inline Texture* GetTextureAsset() { return texture; }

private:
	Texture* texture;
};