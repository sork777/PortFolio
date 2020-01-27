#include "Framework.h"
#include "Widget_Contents.h"
#include "WidgetUtility/EditorHelper.h"


Widget_Contents::Widget_Contents()
	: IWidget()
{
	title = "ContentsBrowser";
	itemSize = 50.0f;
	itemSizeMin = 20.0f;
	itemSizeMax = 100.0f;
	//콘텐츠 폴더 만들기
	rootPath = "../Contents/";
	currentPath = "../Contents/";
	Path::GetDirectoryHierarchy(rootPath, &dirHierarchyRoot);
	contentsRoot = new ContentNode();

	Initialize();
	InitailizeNode(contentsRoot);
	UpdateItems(currentPath);
}

Widget_Contents::~Widget_Contents()
{
}

void Widget_Contents::Initialize()
{
	icons.emplace_back("../Icon/Icon_Audio.png", IconProvider::Get().Load("Icon/Icon_Audio.png", "../", IconType::Audio));
	icons.emplace_back("../Icon/Icon_Mesh.png", IconProvider::Get().Load("Icon/Icon_Mesh.png", "../", IconType::Mesh));
	icons.emplace_back("../Icon/Icon_Animation.png", IconProvider::Get().Load("Icon/Icon_Animation.png", "../", IconType::Anim));
	icons.emplace_back("../Icon/Icon_HeightMap.png", IconProvider::Get().Load("Icon/Icon_HeightMap.png", "../", IconType::HeightMap));
	icons.emplace_back("../Icon/Icon_Model.png", IconProvider::Get().Load("Icon/Icon_Model.png", "../", IconType::Model));
	icons.emplace_back("../Icon/Icon_Folder.png", IconProvider::Get().Load("Icon/Icon_Folder.png", "../", IconType::Folder));
	icons.emplace_back("../Icon/Icon_File.png", IconProvider::Get().Load("Icon/Icon_File.png", "../", IconType::File));
	icons.emplace_back("../Icon/Icon_Custom.png", IconProvider::Get().Load("Icon/Icon_Custom.png", "../", IconType::Custom));

	
	MovableTypes.emplace_back(DragDropPayloadType::Unknown);
	MovableTypes.emplace_back(DragDropPayloadType::Audio);
	MovableTypes.emplace_back(DragDropPayloadType::Mesh);
	MovableTypes.emplace_back(DragDropPayloadType::Anim);
	MovableTypes.emplace_back(DragDropPayloadType::Model);
	MovableTypes.emplace_back(DragDropPayloadType::HeightMap);
	MovableTypes.emplace_back(DragDropPayloadType::Texture);
	MovableTypes.emplace_back(DragDropPayloadType::Folder);
	MovableTypes.emplace_back(DragDropPayloadType::File);
}

void Widget_Contents::Render()
{
	if (ImGui::Button("Import"))
	{
		if (ImportFunc != NULL)
		{
			ImportFunc(L"");
			dirHierarchyRoot.Children.clear();
			dirHierarchyRoot.Children.shrink_to_fit();
			Path::GetDirectoryHierarchy(rootPath, &dirHierarchyRoot);
			UpdateItems(currentPath);
		}
	}

	ImGui::SameLine();
	ImGui::Text(currentPath.c_str());
	ImGui::SameLine(ImGui::GetWindowContentRegionWidth() * 0.8f);
	ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.207f);
	ImGui::SliderFloat("##ItemSize", &itemSize, itemSizeMin, itemSizeMax);
	ImGui::PopItemWidth();

	ImGui::Separator();

	DirectoryViewer();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ShowItems();
	ImGui::EndGroup();

}

void Widget_Contents::ShowItems()
{
	auto PushStyle = []()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 18.0f));
	};

	auto PopStyle = []() { ImGui::PopStyleVar(2); };

	ImVec2 contentSize = 
		ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
	contentSize.x -= 200.0f;
	PushStyle();

	ImGui::BeginChild("##Child", contentSize, true);
	{
		int columns = static_cast<int>(ImGui::GetWindowContentRegionWidth() / itemSize);
		columns = columns < 1 ? 1 : columns;
		ImGui::Columns(columns, nullptr, false);
		for (auto& item : items)
		{
			ImGui::PushID(&item);
			//ImageButton
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton(item.thumbnail.texture->SRV(), ImVec2(itemSize, itemSize-10)))
			{
				//TODO :
			}
			ImGui::PopStyleColor(2);

			DragDrop::SetDragDropPayload(GetPayloadType(item.thumbnail.type), item);
			//DragDrop::SetDragDropPayload(DragDropPayloadType::Texture, item.label);

			//Label
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - itemSize);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + itemSize);
			ImGui::PushItemWidth(itemSize + 8.5f);
			{
				ImGui::TextWrapped(item.label.c_str());
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

void Widget_Contents::DirectoryViewer()
{
	ImVec2 contentSize =
		ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
	ImGui::BeginGroup();
	{
		ImGui::BeginChild("##DirectoryViewer", ImVec2(200, contentSize.y), true);
		{
			int index = 0;
			ChildDirViewer(&dirHierarchyRoot, index);
		}
		ImGui::EndChild();
	}ImGui::EndGroup();
}

void Widget_Contents::ChildDirViewer(DirectoryNode * node, int& index)
{
	auto childs = node->Children;

	ImGuiTreeNodeFlags flags = childs.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (index == selectedDirectory)
		flags |= ImGuiTreeNodeFlags_Selected;

	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
	ImGui::PushItemWidth(200);

	if (ImGui::TreeNodeEx(String::ToString(node->DirectoryName).c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			selectedDirectory = index;
			UpdateItems(String::ToString( node->DirectoryPath));
		}

		Item* item = DragDrop::GetDragDropPayload_Item(MovableTypes);
		if (item != NULL )
		{
			//폴더 자신의 위치나 파일이 본래 있던 폴더가 아닌경우만 드롭 이벤트
			if (item->label != String::ToString(node->DirectoryName)
				&& Path::GetDirectDirectoryName(item->filePath) != String::ToString(node->DirectoryName))
			{
				//TODO:아이템 드롭후 할 행동
				int a = 0;
			}
		}
		for (auto child : childs)
		{
			index++;
			ChildDirViewer(&child, index);
		}
		ImGui::TreePop();
	}
	ImGui::PopItemWidth();
	ImGui::PopStyleVar();
}

void Widget_Contents::InitailizeNode(ContentNode * root)
{

}

void Widget_Contents::UpdateItems(const string & path)
{
	if (!Path::ExistDirectory(path))
	{
		Path::CreateFolder(path);
	}

	items.clear();
	items.shrink_to_fit();

	vector<string> files;
	Path::GetFiles(&files, path, "*.*", false,true);
	for (const auto& file : files)
	{
		SelectIconFromFile(file);
	}
}

void Widget_Contents::SelectIconFromFile(const string & path)
{
	Thumbnail thumbnail;

	if (Path::IsSupportAudioFile(path))
	{
		thumbnail = icons[0].thumbnail;
	}
	else if (Path::IsSupportMeshFile(path))
	{
		thumbnail = icons[1].thumbnail;
	}
	else if (Path::IsConvertedAnimationFile(path))
	{
		thumbnail = icons[2].thumbnail;
	}
	else if (Path::IsSupportMapFile(path))
	{
		thumbnail = icons[3].thumbnail;
	}
	else if (Path::IsConvertedModelFile(path))
	{
		thumbnail = icons[4].thumbnail;
	}
	else if (Path::IsSupportTextureFile(path))
	{
		thumbnail = IconProvider::Get().Load(path, "", IconType::Texture);
	}
	else if (Path::IsDirectory(path))
	{
		thumbnail = icons[5].thumbnail;
	}
	else
	{
		thumbnail = icons[6].thumbnail;
	}
	// 집어넣기
	{
		items.emplace_back(path,thumbnail);
	}
}

DragDropPayloadType  Widget_Contents::GetPayloadType(IconType &type)
{
	DragDropPayloadType returntype;
	switch (type)
	{
	case IconType::Anim:
		returntype = DragDropPayloadType::Anim;
		break;
	case IconType::Audio:
		returntype = DragDropPayloadType::Audio;
		break;
	case IconType::Texture:
		returntype = DragDropPayloadType::Texture;
		break;
	case IconType::HeightMap:
		returntype = DragDropPayloadType::HeightMap;
		break;
	case IconType::Mesh:
		returntype = DragDropPayloadType::Mesh;
		break;
	case IconType::Model:
		returntype = DragDropPayloadType::Model;
		break;
	case IconType::File:
		returntype = DragDropPayloadType::File;
		break;
	case IconType::Folder:
		returntype = DragDropPayloadType::Folder;
		break;
	default:
		returntype = DragDropPayloadType::Unknown;
		break;
	}
	return returntype;
}
