#pragma once

#include "Framework.h"

class IWidget
{
public:
	IWidget()
		: title("")
		, xMin(0.0f)
		, yMin(0.0f)
		, xMax(FLT_MAX)
		, yMax(FLT_MAX)
		, height(0.0f)
		, windowFlags(ImGuiWindowFlags_NoCollapse)
		, bVisible(true)
	{
	}

	virtual ~IWidget() {}

	virtual void Begin()
	{
		if (!bVisible)
			return;

		ImGui::SetNextWindowSize(ImVec2(xMin, yMin), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints
		(
			ImVec2(xMin, yMin),
			ImVec2(xMax, yMax)
		);

		ImGui::Begin(title.c_str(), &bVisible, windowFlags);
	}

	virtual void Render() = 0;

	virtual void End()
	{
		if (!bVisible)
			return;

		height = ImGui::GetWindowHeight();
		ImGui::End();
	}

	const bool& IsVisible() const { return bVisible; }
	void SetIsVisible(const bool& bVisible) { this->bVisible = bVisible; }

protected:
	string title;
	float xMin;
	float yMin;
	float xMax;
	float yMax;
	float height;
	int windowFlags;
	bool bVisible;
};