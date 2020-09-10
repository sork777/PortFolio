#include "Framework.h"
#include "TerrainBrush.h"
#include "Utilities/Perlin.h"
#include "TerrainLod.h"

TerrainBrush::TerrainBrush(TerrainLod* terrain)
	:curTerrainLod(terrain)
{
	shader = terrain->shader;
	//////////////////////////////////////////////////////////////////////////////
	// Const buffer & shader effects
	//////////////////////////////////////////////////////////////////////////////

	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_TerrainBrush");
	ZeroMemory(&raiseDesc, sizeof(RaiseDesc));
	raiseDesc.Res = Vector2(curTerrainLod->width+1, curTerrainLod->height + 1);
}


TerrainBrush::~TerrainBrush()
{
}

void TerrainBrush::Update()
{
	//브러시 위치 업데이트
	{
		brushPos = curTerrainLod->GetPickedPosition();
		//터레인의 이동한 포지션 받기
		curTerrainLod->GetTransform()->Position(&brushDesc.Location);
		//마우스 위치 더하기
		brushDesc.Location += brushPos;
	}
	if (brushDesc.Type != 0 && brushDesc.Type != 3)
		BrushUpdater(brushPos);
}

void TerrainBrush::Render()
{
	if (sBrushBuffer != NULL)
	{
		brushBuffer->Apply();
		sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());
	}
}

////////////////////////////////////
// Brush
////////////////////////////////////
#pragma region Brush

void TerrainBrush::BrushUpdater(Vector3& position)
{
	int w = (int)(curTerrainLod->width + 1)*0.5f;
	int h = (int)(curTerrainLod->height + 1)*0.5f;

	raiseDesc.Position.x = position.x + w;
	raiseDesc.Position.y = -position.z + h;

	raiseDesc.Box.x = (int)position.x - (int)brushDesc.Range + w;//L
	raiseDesc.Box.y = (int)position.x + (int)brushDesc.Range + w;//R
	raiseDesc.Box.z = -(int)position.z - (int)brushDesc.Range + h;//B
	raiseDesc.Box.w = -(int)position.z + (int)brushDesc.Range + h;//T

	if (raiseDesc.Position.x < 0) raiseDesc.Position.x = 0;
	if (raiseDesc.Position.x >= w * 2.0f) raiseDesc.Position.x = w * 2.0f;
	if (raiseDesc.Position.y >= h * 2.0f) raiseDesc.Position.y = h * 2.0f;
	if (raiseDesc.Position.y < 0) raiseDesc.Position.y = 0;

	if (raiseDesc.Box.x < 0) raiseDesc.Box.x = 0;
	if (raiseDesc.Box.y >= w * 2.0f) raiseDesc.Box.y = w * 2.0f;
	if (raiseDesc.Box.z < 0) raiseDesc.Box.z = 0;
	if (raiseDesc.Box.w >= h * 2.0f) raiseDesc.Box.w = h * 2.0f;

}

void TerrainBrush::BrushProperty(const bool& bSplat, bool* bRangeChanged)
{
	ImGui::Text("BrushPosition : %.2f,%.2f,%.2f", brushPos.x, brushPos.y, brushPos.z);

	if (ImGui::CollapsingHeader("BrushParts", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (int i = 0; i < 3; i++)
		{
			string label = "Layer" + to_string(i + 1) + "Texture";
			ImGui::RadioButton(label.c_str(), &raiseDesc.SplattingLayer, i);
		}
		//콤보박스 브러시 타입
		const char* brushItems[] = { "None", "Quad", "Circle", "Slope" };
		static const char* current_item = "None";

		if (ImGui::BeginCombo("##BrushType", current_item))
		{
			//스플래팅인 경우 사이즈 줄여서 목록에서 경사 없애기
			int size = IM_ARRAYSIZE(brushItems);
			if (bSplat) size--;
			for (int n = 0; n < size; n++)
			{
				bool is_selected = (current_item == brushItems[n]);
				if (ImGui::Selectable(brushItems[n], is_selected))
				{
					current_item = brushItems[n];
					raiseDesc.BrushType = n;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		brushDesc.Type = raiseDesc.BrushType;


		if (raiseDesc.BrushType == 0)
		{
		}
		else if (raiseDesc.BrushType < 3)
		{
			ImGui::ColorEdit3("BrushColor", (float*)& brushDesc.Color);
			*bRangeChanged = ImGui::InputInt("BrushRange", (int*)& brushDesc.Range);
			raiseDesc.Radius = brushDesc.Range;

			if (bSplat == false)
			{
				//상승 타입 콤보박스
				const char* raiseTypes[] = { "Increase", "Decrease", "Flatting", "Noise","Smoothing" };
				static const char* current_item = "Increase";

				if (ImGui::BeginCombo("##RaiseType", current_item))
				{
					for (int n = 0; n < IM_ARRAYSIZE(raiseTypes); n++)
					{
						bool is_selected = (current_item == raiseTypes[n]);
						if (ImGui::Selectable(raiseTypes[n], is_selected))
						{
							current_item = raiseTypes[n];
							raiseDesc.RaiseType = n;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				ImGui::SliderFloat("RaiseRate", &raiseRate, 1.0f, 1000.0f);
				raiseDesc.Rate = raiseRate;

				if (raiseDesc.BrushType == 2)
				{
					ImGui::SliderFloat("BrushFactor", &rfactor, 0.1f, 10.0f);
					raiseDesc.Factor = rfactor;
				}
			}
			else if (bSplat)
			{
				ImGui::SliderFloat("BrushGrad", &gfactor, 0.0f, 1000.0f);
				raiseDesc.Factor = gfactor;
			}
		}
		else if (raiseDesc.BrushType == 3)
		{
			bool bSlopX = raiseDesc.SlopDir == 1 ? true : false;

			ImGui::Checkbox("SlopX", &bSlopX);
			raiseDesc.SlopDir = bSlopX ? 1 : 0;
			ImGui::SliderFloat("SlopeAngle", &sAngle, 0.0f, Math::ToRadian(80));
			raiseDesc.Factor = sAngle;
		}
	}

}


#pragma endregion
