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
	1. �̹��� ��ư
	> ������ ��������
	2. ���� ���Ϸ� ����/�ҷ�����
	3. ���� �Ŵ����� ����ϱ�	

	TODO: ���� ����/�ҷ�����, ���� Ž�� �� �ߺ���� ����
*/
class ContentsAsset
{
public:
	ContentsAsset();
	virtual ~ContentsAsset();
	
	// �ʱ�ȭ�� ���� ��ư�� srv ���
	virtual void CreateButtonImage() abstract;
	// ��� ������ Asset�� ���ϵ� Ȯ���ڷ� ��������.
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
