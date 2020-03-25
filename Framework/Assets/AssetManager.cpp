#include "Framework.h"
#include "AssetManager.h"

AssetManager* AssetManager::instance = NULL;

AssetManager * AssetManager::Get()
{
	if (instance == NULL)
		Create();
	return instance;
}

void AssetManager::Create()
{
	if (instance == NULL)
		instance = new AssetManager();
}

void AssetManager::Delete()
{
	SafeDelete(instance);
}

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
	registedAsset.clear();
	registedAsset.shrink_to_fit();
}

void AssetManager::RegistAsset(ContentsAsset * asset)
{
	registedAsset.emplace_back(asset);
	////타입에 따라 다른애들 분류..
	//if (ContentsAssetType::Model == asset->Type())
	//	registedModelAsset.emplace_back(asset);
	//else if (ContentsAssetType::Texture == asset->Type())
	//	registedTextureAsset.emplace_back(asset);
}

void AssetManager::UnRegistAsset(ContentsAsset * asset)
{
	int index = 0;
	for (ContentsAsset* child : registedAsset)
	{
		if (asset == child)
		{
			registedAsset.erase(registedAsset.begin() + index);
			break;
		}
		index++;
	}
}

const bool& AssetManager::ShowAssets()
{
	bool bSelected = false;
	

	ImGui::Begin("ShowAssets");
	{
		auto PushStyle = []()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 18.0f));
		};

		auto PopStyle = []() { ImGui::PopStyleVar(2); };

		//ImVec2 contentSize =
		//	ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
		// contentSize.x -= 200.0f;
		PushStyle();

		float itemSize = 80.0f;

		ImGui::BeginChild("ShowAssets");
		{
			int columns = static_cast<int>(ImGui::GetWindowContentRegionWidth() / itemSize);
			columns = columns < 1 ? 1 : columns;
			ImGui::Columns(columns, nullptr, false);
			for (UINT i = 0; i < registedAsset.size(); i++)
			{
				ImGui::PushID(registedAsset[i]);
				//ImageButton
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				bSelected |= registedAsset[i]->AssetImageButton();
				ImGui::PopStyleColor(2);

				//Label
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - itemSize);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + itemSize);
				ImGui::PushItemWidth(itemSize + 8.5f);
				{
					ImGui::TextWrapped(String::ToString(registedAsset[i]->AssetName()).c_str());
				}
				ImGui::PopItemWidth();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + itemSize);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - itemSize);

				ImGui::NextColumn();
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
		PopStyle();
	}
	ImGui::End();
	return bSelected;
}

ContentsAsset * AssetManager::ViewAssets(const ContentsAssetType & type)
{
	ContentsAsset* returnAsset = NULL;
	for (UINT i = 0; i < registedAsset.size(); i++)
	{
		// 타입 다르면
		int typecheck = (int)type & (int)registedAsset[i]->Type();
		// 타입이 전부 없으면
		if (typecheck == 0)
			continue;


		ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
		//윈도우 사이즈
		ImVec2 wSize = ImGui::GetWindowSize();
		if (wSize.y < pos.y) return false;
		if (wSize.x < pos.x) return false;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;

		if (i == selectedAsset)
			flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

		if (ImGui::TreeNodeEx(String::ToString(registedAsset[i]->AssetName()).c_str(), flags))
		{
			if (ImGui::IsItemClicked())
			{
				selectedAsset = i;
				returnAsset = registedAsset[i];
			}
			ImGui::TreePop();
		}
		ImGui::PopStyleVar();
	}

	return returnAsset;
}


//
//ModelAsset* AssetManager::ViewModelAsset()
//{
//	ModelAsset* returnAsset = NULL;
//
//	for (UINT i = 0; i < registedModelAsset.size(); i++)
//	{
//		ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
//		//윈도우 사이즈
//		ImVec2 wSize = ImGui::GetWindowSize();
//		if (wSize.y < pos.y) return false;
//		if (wSize.x < pos.x) return false;
//
//		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
//
//		if (i == selectedModelAsset)
//			flags |= ImGuiTreeNodeFlags_Selected;
//		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
//
//		if (ImGui::TreeNodeEx(String::ToString(registedModelAsset[i]->AssetName()).c_str(), flags))
//		{
//			if (ImGui::IsItemClicked())
//			{
//				selectedModelAsset = i;
//				returnAsset = registedModelAsset[i];
//			}
//			ImGui::TreePop();
//		}
//		ImGui::PopStyleVar();
//	}
//
//	return returnAsset;
//}
//
//TextureAsset* AssetManager::ViewTextureAsset()
//{
//	TextureAsset* returnAsset = NULL;
//
//	for (UINT i = 0; i < registedTextureAsset.size(); i++)
//	{
//		ImVec2 pos = ImGui::GetItemRectMin() - ImGui::GetWindowPos();
//		//윈도우 사이즈
//		ImVec2 wSize = ImGui::GetWindowSize();
//		if (wSize.y < pos.y) return false;
//		if (wSize.x < pos.x) return false;
//
//		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
//
//		if (i == selectedTextureAsset)
//			flags |= ImGuiTreeNodeFlags_Selected;
//		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
//
//		if (ImGui::TreeNodeEx(String::ToString(registedTextureAsset[i]->AssetName()).c_str(), flags))
//		{
//			if (ImGui::IsItemClicked())
//			{
//				selectedTextureAsset = i;
//				returnAsset = registedTextureAsset[i];
//			}
//			ImGui::TreePop();
//		}
//		ImGui::PopStyleVar();
//	}
//
//	return returnAsset;
//}
