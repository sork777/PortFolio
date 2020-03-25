#include "Framework.h"
#include "TextureAsset.h"


TextureAsset::TextureAsset(Texture * texture)
	:texture(texture)
{
	assetType = ContentsAssetType::Texture;
	assetName = Path::GetFileNameWithoutExtension(texture->GetFile());
	CreateButtonImage();
}

TextureAsset::TextureAsset(const wstring& path, const wstring& dir)
{
	assetType = ContentsAssetType::Texture;
	texture = new Texture(path, dir);
	assetName = Path::GetFileNameWithoutExtension( texture->GetFile());
	CreateButtonImage();
}

TextureAsset::~TextureAsset()
{
}

void TextureAsset::CreateButtonImage()
{
	if (true == bCreateButton)
		return;
	buttonSrv = texture->SRV();
	bCreateButton = true;
}

void TextureAsset::SaveAssetContents()
{
	//텍스쳐 관련 에셋 저장 정보.
	texture->GetFile();
	texture->GetDir();
	
}

void TextureAsset::LoadAssetContents()
{
}