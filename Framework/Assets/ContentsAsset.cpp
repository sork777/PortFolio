#include "Framework.h"
#include "ContentsAsset.h"

ContentsAsset::ContentsAsset()
	:buttonSrv(NULL)
{
	AssetManager::Get()->RegistAsset(this);
}

ContentsAsset::~ContentsAsset()
{
	SafeRelease(buttonSrv);
	AssetManager::Get()->UnRegistAsset(this);
}

const bool& ContentsAsset::AssetImageButton()
{
	// �ش� ������ �����ִ� ��ư Ŭ�� ���� ����
	bool bSelected = ImGui::ImageButton(buttonSrv, ImVec2(80, 80));

	return bSelected;
}
