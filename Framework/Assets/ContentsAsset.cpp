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
	// 해당 에셋을 보여주는 버튼 클릭 여부 리턴
	bool bSelected = ImGui::ImageButton(buttonSrv, ImVec2(80, 80));

	return bSelected;
}
