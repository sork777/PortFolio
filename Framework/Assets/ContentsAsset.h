#pragma once

enum class ContentsAssetType : UINT
{
	None			= 0,
	Model			= 1 << 0, 
	AnimationClip	= 1 << 1,
	Figure			= 1 << 2,
	Texture			= 1 << 3,
	All				= Model| AnimationClip| Figure| Texture,
};

/*
	1. 이미지 버튼
	> 컨텐츠 브라우저용
	2. 에셋 파일로 저장/불러오기
	3. 에셋 매니저에 등록하기	

	TODO: 에셋 저장/불러오기, 에셋 탐색 및 중복등록 방지
*/
class ContentsAsset
{
public:
	ContentsAsset();
	virtual ~ContentsAsset();
	
	// 초기화를 통해 버튼의 srv 사용
	virtual void CreateButtonImage() abstract;
	// 모든 형태의 Asset을 통일된 확장자로 저장하자.
	virtual void SaveAssetContents() abstract;
	virtual void LoadAssetContents() abstract;

public:
	const bool& AssetImageButton();
	inline const ContentsAssetType& Type() { return assetType; }
	inline const wstring& AssetName() { return assetName; }
	inline const wstring& AssetPath() { return assetPath; }
	inline const wstring& AssetDir() { return assetDir; }

protected:
	ContentsAssetType assetType;
	wstring assetName;
	wstring assetPath;
	wstring assetDir;

	ID3D11ShaderResourceView* buttonSrv;
	bool bCreateButton = false;
};
