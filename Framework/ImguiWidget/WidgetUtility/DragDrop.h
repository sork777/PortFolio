#pragma once
#include "Framework.h"
#include "EditorHelper.h"

enum class DragDropPayloadType : UINT
{
	Unknown,
	Audio,
	Mesh,
	Anim,
	Model,
	HeightMap,
	Texture,
	Folder,
	File,
	//
	//Texture,
	//Object,
	//Model,
	//Audio,
	//Script,
	//

};

class DragDrop
{
public:
	static void SetDragDropPayload(const DragDropPayloadType& type, const Item& data)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload
			(
				reinterpret_cast<const char*>(&type),
				&data,
				sizeof(data),
				ImGuiCond_Once
			);
			ImGui::Text(data.label.c_str());
			ImGui::EndDragDropSource();
		}
	}
	// 옮길 녀석의 아래쪽에 위치시킬것
	static Item* GetDragDropPayload_Item(const DragDropPayloadType& type)
	{
		vector<DragDropPayloadType> types;
		types.emplace_back(type);

		return GetDragDropPayload_Item(types);
	}

	static Item* GetDragDropPayload_Item(const vector<DragDropPayloadType>& types)
	{
		for (DragDropPayloadType type : types)
		{
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload
				(
					reinterpret_cast<const char*>(&type)
					//, target_flags
				);

				if (payload)
					return reinterpret_cast<Item*>(payload->Data);

				ImGui::EndDragDropTarget();
			}
		}
		return NULL;
	}
};