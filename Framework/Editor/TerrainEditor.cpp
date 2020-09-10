#include "Framework.h"
#include "Utilities/Perlin.h"
#include "../Environment/Terrain/TerrainLod.h"
#include "../Environment/Terrain/TerrainBrush.h"
#include "../Environment/BillBoard.h"
#include "TerrainEditor.h"


TerrainEditor::TerrainEditor(TerrainLod* terrainLod)
	:curTerrainLod(terrainLod)
	, BrushedArea(+FLT_MAX, -0, +FLT_MAX, -0)
{
	rShader = terrainLod->shader;	
	HMapSrv = terrainLod->HMapSrv;
	Initialize();
}


TerrainEditor::~TerrainEditor()
{
}

void TerrainEditor::Initialize()
{
	brush = new TerrainBrush(curTerrainLod);
	lineColorBuffer = new ConstantBuffer(&lineColorDesc, sizeof(LineColorDesc));
	sLineColorBuffer = rShader->AsConstantBuffer("CB_GridLine");
	UINT width = curTerrainLod->width + 1;
	UINT height = curTerrainLod->height + 1;
	raiseCS = SETSHADER(L"HW08_TerrainLoDBrush.fx");
	raiseCT[0] = new CsTexture(width,height,DXGI_FORMAT_R16G16B16A16_TYPELESS);
	raiseCT[1] = new CsTexture(width,height, DXGI_FORMAT_R16G16B16A16_TYPELESS);
	raiseDesc = &brush->raiseDesc;
	raiseBuffer = new ConstantBuffer(&brush->raiseDesc, sizeof(RaiseDesc));
	sRaiseBuffer = raiseCS->AsConstantBuffer("CB_Raise");

	lineColorDesc.Size = curTerrainLod->info.CellSpacing*2.0f;

	LoadPerlinMap();
}

void TerrainEditor::Destroy()
{
	SafeDelete(raiseBuffer);
	SafeRelease(sRaiseBuffer);

	SafeDelete(raiseCT[0]);
	SafeDelete(raiseCT[1]);
}

void TerrainEditor::Update()
{
	//높이맵 변화 끄기
	curTerrainLod->SetHMapCond(false);

	brush->Update();
	Vector3 brushPos = brush->brushPos;
	//레이즈 여부 초기값은 false로
	bool bRaise = false;
	bool bNoise = raiseDesc->RaiseType == 3 ? true : false;
	bool bSmooth = raiseDesc->RaiseType == 4 ? true : false;
	
	//TODO: 옮길곳 없나...
	if (bSplat == true && Keyboard::Get()->Press(VK_SHIFT))
	{
		//스플래팅
		if (Mouse::Get()->Press(0))
		{
			if (raiseDesc->BrushType == 1)
				UpdateAlphaMap(0, 1);
			else if (raiseDesc->BrushType == 2)
				UpdateAlphaMap(1, 1);
		}
	}
	if (raiseDesc->BrushType > 0)
	{
		if (bSplat == false && Keyboard::Get()->Press(VK_SHIFT))
		{
			if (Mouse::Get()->Press(0))
			{
				if (raiseDesc->BrushType == 1)
				{
					bNoise ? HeightNoise() :
						bSmooth ? HeightSmoothing() : RaiseHeightQuad();
				}
				if (raiseDesc->BrushType == 2)
				{
					bNoise ? HeightNoise() :
						bSmooth ? HeightSmoothing() : RaiseHeightCircle();
				}
				bRaise = true;
			}
			//press는 누른 상태라서 down으로
			if (Mouse::Get()->Down(0))
			{
				if (raiseDesc->BrushType == 3)
				{
					if (brushPos.y >= 0.0f)
						if (bSlope == false)
						{
							//시작점 설정
							slopBox.x = brushPos.x;
							slopBox.y = brushPos.z;
							bSlope = true;
						}
						else
						{
							//끝점 설정후 계산
							slopBox.z = brushPos.x;
							slopBox.w = brushPos.z;
							RaiseHeightSlope();
						}
				}
				else
					bSlope = false;
				bRaise = true;

			}
		}

		//브러시가 이동하면서 올린 장소들을 커버할 영역 설정
		if (bRaise == true || bSlope)
		{
			BrushedArea.x = raiseDesc->Box.x < BrushedArea.x ? raiseDesc->Box.x : BrushedArea.x;
			BrushedArea.y = raiseDesc->Box.y > BrushedArea.y ? raiseDesc->Box.y : BrushedArea.y;
			BrushedArea.z = raiseDesc->Box.z < BrushedArea.z ? raiseDesc->Box.z : BrushedArea.z;
			BrushedArea.w = raiseDesc->Box.w > BrushedArea.w ? raiseDesc->Box.w : BrushedArea.w;
		}

		if (Keyboard::Get()->Up(VK_SHIFT))
		{
			if (bSlope == true)
			{
				slopBox = Vector4(+FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX);
				bSlope = false;
			}
			curTerrainLod->UpdateQuadHeight(BrushedArea);
			BrushedArea = Vector4(+FLT_MAX, -0, +FLT_MAX, -0);
		}
		else if (Mouse::Get()->Up(0))
		{
			if (bSlope == false)
			{
				curTerrainLod->UpdateQuadHeight(BrushedArea);
				BrushedArea = Vector4(+FLT_MAX, -0, +FLT_MAX, -0);
			}
		}

	}
	curTerrainLod->Update();
}

void TerrainEditor::PreRender()
{
	curTerrainLod->PreRender();
}

void TerrainEditor::Render()
{
	if (raiseDesc->RaiseType == 3)
	{
		perlinGen->PerlinController();
		perlinGen->GeneratorNoise2D();
	}
	brush->Render();
	if (sLineColorBuffer != NULL)
	{
		lineColorBuffer->Apply();
		sLineColorBuffer->SetConstantBuffer(lineColorBuffer->Buffer());
	}
	curTerrainLod->Pass(bWire ? 1 : 0);
	curTerrainLod->Render();

	if (true == bQuadFrame)
	{
		curTerrainLod->QuadTreeRender();
	}
}

void TerrainEditor::PostRender()
{
}

void TerrainEditor::RaiseHeightQuad()
{
	UpdateAlphaMap(0);
}

void TerrainEditor::RaiseHeightCircle()
{
	UpdateAlphaMap(1);
}

void TerrainEditor::RaiseHeightSlope()
{
	bool bReverse = false;
	raiseDesc->SlopRev = 0;
	bool slopX = raiseDesc->SlopDir;

	int w = (int)(curTerrainLod->width + 1)*0.5f;
	int h = (int)(curTerrainLod->height + 1)*0.5f;
	//박스 만들기 위해 위치 조정
	raiseDesc->Box.x = (int)slopBox.x + w;
	raiseDesc->Box.y = (int)slopBox.z + w;
	raiseDesc->Box.z = -(int)slopBox.y + h;
	raiseDesc->Box.w = -(int)slopBox.w + h;

	if (raiseDesc->Box.x > raiseDesc->Box.y)
	{
		swap(raiseDesc->Box.x, raiseDesc->Box.y);
		if (slopX == true)
			bReverse = true;
	}
	if (raiseDesc->Box.z > raiseDesc->Box.w)
	{
		swap(raiseDesc->Box.z, raiseDesc->Box.w);
		if (slopX == false)
			bReverse = true;
	}

	if (raiseDesc->Box.x < 0) raiseDesc->Box.x = 0;
	if (raiseDesc->Box.y >= w * 2.0f) raiseDesc->Box.y = w * 2.0f;
	if (raiseDesc->Box.z < 0) raiseDesc->Box.z = 0;
	if (raiseDesc->Box.w >= h * 2.0f) raiseDesc->Box.w = h * 2.0f;
	if (bReverse) raiseDesc->SlopRev = 1;

	UpdateAlphaMap(2);
}

void TerrainEditor::HeightNoise()
{
	if (perlinGen->CanUsing() == false)
		return;
	UpdateAlphaMap(1, 2);
}

void TerrainEditor::HeightSmoothing()
{
	UpdateAlphaMap(0, 2);
}

void TerrainEditor::LoadPerlinMap()
{
	perlinGen = new Perlin(0, 0, DXGI_FORMAT_R16G16B16A16_TYPELESS);
	raiseCS->AsSRV("PerlinMap")->SetResource(perlinGen->GetPerlinSrv());
}

void TerrainEditor::UpdateAlphaMap(UINT pass, UINT tech)
{
	//높이맵 변화 켜기

	raiseBuffer->Apply();
	sRaiseBuffer->SetConstantBuffer(raiseBuffer->Buffer());

	static bool bMethodInodd = false;
	raiseCS->AsSRV("AlphaMap")->SetResource(HMapSrv);
	raiseCS->AsUAV("OutputMap")->SetUnorderedAccessView(raiseCT[bMethodInodd ? 0 : 1]->UAV());
	raiseCS->Dispatch(tech, pass, ceil(curTerrainLod->AlphaMapPixel.size() / 1024.0f), 1, 1);
	HMapSrv = raiseCT[bMethodInodd ? 0 : 1]->SRV();
	bMethodInodd = !bMethodInodd;
	curTerrainLod->sAlphaTexture->SetResource(HMapSrv);
	curTerrainLod->HMapSrv = HMapSrv;
	curTerrainLod->SetHMapCond(true);

	// 업데이트 영역의 높이 픽셀의 정리
	ID3D11Texture2D* srcTexture;
	HMapSrv->GetResource((ID3D11Resource **)&srcTexture);

	D3D11_TEXTURE2D_DESC srcDesc;
	srcTexture->GetDesc(&srcDesc);


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = srcDesc.Width;
	desc.Height = srcDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.SampleDesc = srcDesc.SampleDesc;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;


	ID3D11Texture2D* texture;
	Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture));
	Check(D3DX11LoadTextureFromTexture(D3D::GetDC(), srcTexture, NULL, texture));

	UINT* colors = new UINT[desc.Width * desc.Height];
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(texture, 0, D3D11_MAP_READ, NULL, &subResource);
	{
		memcpy(colors, subResource.pData, sizeof(UINT) * desc.Width * desc.Height);
	}
	D3D::GetDC()->Unmap(texture, 0);


	for (int z = raiseDesc->Box.z; z < raiseDesc->Box.w; z++)
	{
		for (int x = raiseDesc->Box.x; x < raiseDesc->Box.y; x++)
		{
			UINT index = desc.Width * z + x;

			CONST FLOAT f = 1.0f / 255.0f;

			float r = (float)((0xFF000000 & colors[index]) >> 24);
			float g = (float)((0x00FF0000 & colors[index]) >> 16);
			float b = (float)((0x0000FF00 & colors[index]) >> 8);
			float a = (float)((0x000000FF & colors[index]) >> 0);

			curTerrainLod->AlphaMapPixel[index] = (D3DXCOLOR(a, b, g, r)*f);
		}
	}

	SafeDeleteArray(colors);
	SafeRelease(texture);
}

void TerrainEditor::TerrainController()
{
	ImGui::Begin("Terrain_Controller", &bEditMode);
	{
		if (ImGui::Checkbox("LOD", &bLod))
			curTerrainLod->SetLod(bLod);
		ImGui::SameLine();
		ImGui::Checkbox("WireFrame", &bWire);
		ImGui::SameLine();
		ImGui::Checkbox("QuadFrame", &bQuadFrame);
		ImGui::Checkbox("Splatting", &bSplat);
		ImGui::Separator();
		ImGui::ImageButton(HMapSrv, ImVec2(120, 120));
		{
			if (ImGui::Button("SaveXml", ImVec2(60, 25)))
			{
				curTerrainLod->SaveTerrainToXml();
			}
			ImGui::SameLine(80);
			if (ImGui::Button("LoadXml", ImVec2(60, 25)))
			{
				curTerrainLod->LoadTerrainFromXml();
			}
		}
		ImGui::Separator();

		///////////////////////////////////////////////////////////////////////////
		bool bRangeChanged;
		brush->BrushProperty(bSplat,&bRangeChanged);

		if (raiseDesc->RaiseType == 3 && bRangeChanged == true)
		{
			perlinGen->Resize((UINT)raiseDesc->Radius);
			raiseCS->AsSRV("PerlinMap")->SetResource(perlinGen->GetPerlinSrv());
		}
		ImGui::Separator();

		///////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("LineParts"))
		{
			ImGui::ColorEdit3("LineColor", (float*)& lineColorDesc.Color);
			ImGui::SliderInt("VisibleLine", (int*)&lineColorDesc.Visible, 0, 1);
			int size = lineColorDesc.Size;
			int max = curTerrainLod->info.CellsPerPatch * 2;
			int i = 0;
			while (max > 1)
			{
				max = max >> 1;
				i++;
			}
			max = i;
			i = 0;
			while (size > 1)
			{
				size = size >> 1;
				i++;
			}
			size = i;
			ImGui::SliderInt("LineSize(Powered)", &size, 0, max);
			lineColorDesc.Size = 1 << (size);
			ImGui::Text("LineSize : %d", (int)lineColorDesc.Size);
			ImGui::SliderFloat("LineThickness", &lineColorDesc.Thickness, 0.01f, 1.0f);
		}
		ImGui::Separator();
		///////////////////////////////////////////////////////////////////////////
		bool bChangeAlpha;
		bool bUseAlpha;
		curTerrainLod->TerrainLayerProperty(&bChangeAlpha,&bUseAlpha);

		if (true == bChangeAlpha)
		{
			UINT width = curTerrainLod->width+1;
			UINT height = curTerrainLod->height+1;
			raiseDesc->Res = Vector2(width, height);
			raiseCT[0]->Resize(width, height);
			raiseCT[1]->Resize(width, height);
			HMapSrv = curTerrainLod->alphaTexture->SRV();

			if (bUseAlpha == true)
			{
				UpdateAlphaMap(0, 3);
			}
			else
			{
				UpdateAlphaMap(1, 3);
			}
		}

		ImGui::End();
	}

}

