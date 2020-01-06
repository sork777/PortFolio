#include "Framework.h"
#include "TerrainLod.h"
#include "Viewer/Fixity.h"
#include "Utilities/Perlin.h"
#include "Utilities/Xml.h"


TerrainLod::TerrainLod(InitializeInfo& info)
	:Renderer(info.shader), baseTexture(NULL)
	, BrushedArea(+FLT_MAX, -0, +FLT_MAX, -0)
{
	this->info = info;
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	
	raiseCS = new Shader(L"HW08_TerrainLoDBrush.fx");
	raiseCT[0] = new CsTexture(); 
	raiseCT[1] = new CsTexture(); 
	//////////////////////////////////////////////////////////////////////////////
	// Const buffer & shader effects
	//////////////////////////////////////////////////////////////////////////////
	
	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_TerrainBrush");

	lineColorBuffer = new ConstantBuffer(&lineColorDesc, sizeof(LineColorDesc));
	sLineColorBuffer = shader->AsConstantBuffer("CB_GridLine");

	raiseBuffer = new ConstantBuffer(&raiseDesc, sizeof(RaiseDesc));
	sRaiseBuffer = raiseCS->AsConstantBuffer("CB_Raise");

	texelBuffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sTexelBuffer = shader->AsConstantBuffer("CB_Terrain");

	sBaseTexture = shader->AsSRV("BaseMap");
	sLayerTexture = shader->AsSRV("LayerMaps");
	sNormalTexture = shader->AsSRV("NormalMap");
	sAlphaTexture = shader->AsSRV("HeightMap");
	
	/* 기본 높이맵 호출 - 손 안댄것*/	
	{
		UpdateAlphaMap(0, 3);
		ID3D11Texture2D* srcTexture;
		HMapSrv->GetResource((ID3D11Resource **)&srcTexture);
		D3D11_TEXTURE2D_DESC desc;
		srcTexture->GetDesc(&desc);
		width = desc.Width-1;
		height = desc.Height-1;
		raiseDesc.Res = Vector2(width + 1, height + 1);
		Texture::ReadPixels(srcTexture, DXGI_FORMAT_R8G8B8A8_UNORM, &AlphaMapPixel);
	}

	bufferDesc.TexelCellSpaceU = 1.0f / ((float)width);
	bufferDesc.TexelCellSpaceV = 1.0f / ((float)height);
	bufferDesc.HeightRatio = info.HeightRatio;
	
	lineColorDesc.Size = info.CellSpacing*2.0f;

	patchVertexRows = (width / info.CellsPerPatch) + 1;
	patchVertexCols = (height / info.CellsPerPatch) + 1;

	vertexCount = patchVertexRows * patchVertexCols;
	faceCount = (patchVertexRows - 1) * (patchVertexCols - 1);
	indexCount = faceCount * 4;

	LoadPerlinMap();
	CalcBoundY();
	CreateVertexData();
	CreateIndexData();

	//else

	QuadTreeNode* root = CreateQuadTreeData(NULL,Vector2(0, 0), Vector2(width, height));
	quadTree = new QuadTree(root);
	
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTerrain));
	indexBuffer = new IndexBuffer(indices, indexCount);


	TestCol = new Collider();
	TestCol->GetTransform()->Scale(1, 3, 1);
	AreaCol = new Collider();
}


TerrainLod::~TerrainLod()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(texelBuffer);

	SafeDelete(baseTexture);
	SafeRelease(sBaseTexture);
	for (int i = 0; i < 3; i++)
	{
		SafeDelete(layerTexture[i]);
	}
	SafeRelease(sLayerTexture);
	SafeDelete(alphaTexture);
	SafeRelease(sAlphaTexture);
	SafeDelete(normalTexture);

	SafeDelete(brushBuffer);
	SafeRelease(sBrushBuffer);
	SafeDelete(raiseBuffer);
	SafeRelease(sRaiseBuffer);

	SafeDelete(raiseCT[0]);
	SafeDelete(raiseCT[1]);
	SafeDelete(raiseCS);
	SafeDelete(shader);
}


void TerrainLod::Update()
{
	
	//레이즈 여부 초기값은 false로
	bool bRaise = false;
	bool bNoise = raiseDesc.RaiseType == 3 ? true : false;
	bool bSmooth = raiseDesc.RaiseType == 4 ? true : false;
	//브러시 위치 업데이트
	{
		brushPos = GetPickedPosition();
		//터레인의 이동한 포지션 받기
		GetTransform()->Position(&brushDesc.Location);
		//마우스 위치 더하기
		brushDesc.Location += brushPos;
	}
	if (brushDesc.Type != 0 && brushDesc.Type != 3)
		BrushUpdater(brushPos);
	if (bSplat == true && Keyboard::Get()->Press(VK_SHIFT))
	{
		//스플래팅
		if (Mouse::Get()->Press(0))
		{
			if (brushDesc.Type == 1)
				UpdateAlphaMap(0, 1);
			else if (brushDesc.Type == 2)
				UpdateAlphaMap(1, 1);
		}
	}
	if (brushDesc.Type > 0)
	{		
		if (bSplat == false && Keyboard::Get()->Press(VK_SHIFT))
		{
			if (Mouse::Get()->Press(0))
			{
				if (brushDesc.Type == 1)
				{
					bNoise?HeightNoise():
					bSmooth ? HeightSmoothing() : RaiseHeightQuad();
				}
				if (brushDesc.Type == 2)
				{
					bNoise ? HeightNoise() :
					bSmooth ? HeightSmoothing() : RaiseHeightCircle();
				}
				bRaise = true;
			}
			//press는 누른 상태라서 down으로
			if (Mouse::Get()->Down(0))
			{
				if (brushDesc.Type == 3)
				{
					if (brushPos.y > 0)
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
		if (bRaise == true || bSlope )
		{
			BrushedArea.x = raiseDesc.Box.x < BrushedArea.x ? raiseDesc.Box.x : BrushedArea.x;
			BrushedArea.y = raiseDesc.Box.y > BrushedArea.y ? raiseDesc.Box.y : BrushedArea.y;
			BrushedArea.z = raiseDesc.Box.z < BrushedArea.z ? raiseDesc.Box.z : BrushedArea.z;
			BrushedArea.w = raiseDesc.Box.w > BrushedArea.w ? raiseDesc.Box.w : BrushedArea.w;
		}

		if (Keyboard::Get()->Up(VK_SHIFT))
		{
			if (bSlope == true)
			{
				slopBox = Vector4(+FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX);
				bSlope = false;
			}
			UpdateQuadHeight();
			BrushedArea = Vector4(+FLT_MAX, -0, +FLT_MAX, -0);
		}
		else if (Mouse::Get()->Up(0))
		{
			if (bSlope == false)
			{
				UpdateQuadHeight();
				BrushedArea = Vector4(+FLT_MAX, -0, +FLT_MAX, -0);
			}
		}
		
	}


	//TODO: 나중 삭제
	if (Mouse::Get()->Press(1) && Keyboard::Get()->Press(VK_SHIFT))
	{
		Vector3 pos = brushPos;
		pos.y = GetPickedHeight() + 1.5f;
		TestCol->GetTransform()->Position(pos);
	}
	Super::Update();
	   	 
}

void TerrainLod::Render()
{
	Super::Render();
	
	if (raiseDesc.RaiseType == 3)
		perlinGen->GeneratorNoise2D();


	if (baseTexture != NULL)
		sBaseTexture->SetResource(baseTexture->SRV());
	if (normalTexture != NULL)
		sNormalTexture->SetResource(normalTexture->SRV());

	{
		sLayerTexture->SetResourceArray(layerViews,0,3);
	}
	if (sBrushBuffer != NULL)
	{
		brushBuffer->Apply();
		sBrushBuffer->SetConstantBuffer(brushBuffer->Buffer());
	}

	if (sLineColorBuffer != NULL)
	{
		lineColorBuffer->Apply();
		sLineColorBuffer->SetConstantBuffer(lineColorBuffer->Buffer());
	}
	texelBuffer->Apply();
	sTexelBuffer->SetConstantBuffer(texelBuffer->Buffer());
	shader->DrawIndexed(Tech() , Pass(), indexCount);

	if (bQuadFrame)
		quadTree->Render(Color(0, 0, 1, 1));
	//TODO: 나중 삭제
	TestCol->Render(Color(1, 1, 0, 1));
	
}

void TerrainLod::ColliderRender()
{

}
////////////////////////////////////
// Textures
////////////////////////////////////
#pragma region Texture

void TerrainLod::BaseTexture(wstring file)
{
	SafeDelete(baseTexture);

	baseTexture = new Texture(file);
	sBaseTexture->SetResource(baseTexture->SRV());
}

void TerrainLod::AlphaTexture(wstring file, bool bUseAlpha)
{
	SafeDelete(alphaTexture);

	alphaTexture = new Texture(file);

	HMapSrv = alphaTexture->SRV();
	//사이즈 변동
	width = this->alphaTexture->GetWidth() - 1;
	height = this->alphaTexture->GetHeight() - 1;
	raiseDesc.Res = Vector2(width + 1, height + 1);
	raiseCT[0]->Resize(width + 1, height + 1);
	raiseCT[1]->Resize(width + 1, height + 1);
	
	if(bUseAlpha == true)
	{
		UpdateAlphaMap(0, 3);
	}
	else
	{
		UpdateAlphaMap(1, 3);		
	}
	ID3D11Texture2D* srcTexture;
	HMapSrv->GetResource((ID3D11Resource **)&srcTexture);
	Texture::ReadPixels(srcTexture, DXGI_FORMAT_R8G8B8A8_UNORM, &AlphaMapPixel);

	SafeDelete(quadTree);
	QuadTreeNode* root = CreateQuadTreeData(NULL, Vector2(0, 0), Vector2(width, height));
	quadTree = new QuadTree(root);
}

void TerrainLod::LayerTextures(wstring layer, UINT layerIndex)
{
	this->layerIndex = layerIndex;
	SafeDelete(layerTexture[layerIndex]);
	layerTexture[layerIndex] = new Texture(layer);
	layerViews[layerIndex] = layerTexture[layerIndex]->SRV();
	sLayerTexture->SetResourceArray(layerViews,0,3);
}

void TerrainLod::NDTexture(wstring normal)
{
	SafeDelete(normalTexture);

	normalTexture = new Texture(normal);
	sNormalTexture->SetResource(normalTexture->SRV());
}

void TerrainLod::SetLayer(UINT layerIndex)
{
	this->layerIndex = layerIndex;
	raiseDesc.SplattingLayer = layerIndex;
}

#pragma endregion

////////////////////////////////////
// Lod Creaters
////////////////////////////////////

bool TerrainLod::InBounds(UINT row, UINT col)
{
	return row >= 0 && row < height && col >= 0 && col < width;
}

void TerrainLod::CalcBoundY()
{
	bounds.assign(faceCount, Vector2());

	for (UINT row = 0; row < patchVertexRows-1; row++)
	{
		for (UINT col = 0; col < patchVertexCols-1; col++)
			CalcPatchBounds(row, col);
	}
}

void TerrainLod::CalcPatchBounds(UINT row, UINT col)
{
	UINT x0 = col * info.CellsPerPatch;
	UINT x1 = (col + 1) * info.CellsPerPatch;

	UINT y0 = row * info.CellsPerPatch;
	UINT y1 = (row + 1) * info.CellsPerPatch;


	float minY = FLT_MAX;
	float maxY = -FLT_MAX;

	for (UINT y = y0; y <= y1; y++)
	{
		for (UINT x = x0; x <= x1; x++)
		{
			float data = 0.0f;
			if (InBounds(y, x))
			{
				data = AlphaMapPixel[y * (width + 1) + x].a;
			}
			
			minY = min(minY, data);
			maxY = max(maxY, data);
		}
	}

	UINT patchID = row * (patchVertexCols - 1) + col;
	bounds[patchID] = Vector2(minY, maxY);
}

void TerrainLod::CreateVertexData()
{
	vertices = new VertexTerrain[vertexCount];

	float halfWidth = 0.5f * (float)width;
	float halfDepth = 0.5f * (float)height;

	float patchWidth = (float)width / (float)(patchVertexCols - 1);
	float patchDepth = (float)height / (float)(patchVertexRows - 1);

	float du = 1.0f / (float)(patchVertexCols - 1);
	float dv = 1.0f / (float)(patchVertexRows - 1);

	for (UINT row = 0; row < patchVertexRows; row++)
	{
		float z = halfDepth - (float)row * patchDepth;
		for (UINT col = 0; col < patchVertexCols; col++)
		{
			float x = -halfWidth + (float)col * patchWidth;
			UINT vertId = row * patchVertexCols + col;

			vertices[vertId].Position = Vector3(x, 0, z);
			vertices[vertId].Uv = Vector2(col * du, row * dv);
		}
	}

	for (UINT row = 0; row < patchVertexRows - 1; row++)
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++)
		{
			UINT patchID = row * (patchVertexCols - 1) + col;
			UINT vertID = row * patchVertexCols + col;

			vertices[vertID].BoundsY = bounds[patchID];
		}
	}
}

void TerrainLod::CreateIndexData()
{
	vector<WORD> indices;
	for (WORD row = 0; row < (WORD)patchVertexRows - 1; row++)
	{
		for (WORD col = 0; col < (WORD)patchVertexCols - 1; col++)
		{
			indices.push_back(row * (WORD)patchVertexCols + col);
			indices.push_back(row * (WORD)patchVertexCols + col + 1);
			indices.push_back((row + 1) * (WORD)patchVertexCols + col);
			indices.push_back((row + 1) * (WORD)patchVertexCols + col + 1);
		}
	}

	this->indices = new UINT[indexCount];
	copy
	(
		indices.begin(), indices.end(),
		stdext::checked_array_iterator<UINT *>(this->indices, indexCount)
	);
}

void TerrainLod::LoadPerlinMap()
{
	perlinGen = new Perlin();	
	raiseCS->AsSRV("PerlinMap")->SetResource(perlinGen->GetPerlinSrv());
}

////////////////////////////////////
// Brush
////////////////////////////////////
#pragma region Brush

Vector3 TerrainLod::GetPickedPosition()
{
	
	Matrix world = GetTransform()->World();
	Vector3 start, direction;
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

	Vector3 mouse = Mouse::Get()->GetPosition();

	Vector3 n, f;//근면 원면

	//근면
	mouse.z = 0.0f;
	Context::Get()->GetViewport()->Unproject(&n, mouse, world, V, P);

	//원면
	mouse.z = 1.0f;
	Context::Get()->GetViewport()->Unproject(&f, mouse, world, V, P);

	//n에서 f로 쏜 방향
	direction = f - n;
	start = n;
	static QuadTreeNode* selectedQNode = NULL;
	selectedQNode=quadTree->GetPickedNode(start,direction);
	//선택노드 없으면 리턴
	if (selectedQNode == NULL)
		return Vector3(-1, -1, -1);

	return selectedQNode->GetCollider()->GetSelectPos();
}

float TerrainLod::GetPickedHeight()
{
	int w = (int)(width)*0.5f;
	int h = (int)(height)*0.5f;
	int x = (int)ceil(brushPos.x) + w;
	int z = -(int)ceil(brushPos.z) + h;

	if (x < 0 || x >= width) return -1.0f;
	if (z < 0 || z >= height) return -1.0f;
	UINT index[4];
	index[0] = (width+1) * z + x;
	index[1] = (width+1) * (z + 1) + x;
	index[2] = (width+1) * z + x + 1;
	index[3] = (width+1) * (z + 1) + x + 1; 
	Vector3 p[4];
	for (int i = 0; i < 4; i++)
	{
		if (index[i] >= AlphaMapPixel.size()) return -1.0f;
	}
	p[0] = Vector3(x, AlphaMapPixel[index[0]].a*bufferDesc.HeightRatio, z);
	p[1] = Vector3(x, AlphaMapPixel[index[1]].a*bufferDesc.HeightRatio, z+1);
	p[2] = Vector3(x+1, AlphaMapPixel[index[2]].a*bufferDesc.HeightRatio, z);
	p[3] = Vector3(x+1, AlphaMapPixel[index[3]].a*bufferDesc.HeightRatio, z+1);
	float u, v, distance;

	//빛의 시작점
	Vector3 start(x, 1000, z);
	//빛을 쏠 방향
	Vector3 direction(0, -1, 0);

	//충돌 결과, 없으면 초기값
	Vector3 result(-1, -1, -1);

	if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;

	if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;

	return result.y;
}

void TerrainLod::BrushUpdater(Vector3& position)
{
	int w = (int)(width+1)*0.5f;
	int h = (int)(height+1)*0.5f;

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

void TerrainLod::RaiseHeightQuad()
{
	UpdateAlphaMap(0);
}

void TerrainLod::RaiseHeightCircle()
{
	UpdateAlphaMap(1);
}

void TerrainLod::RaiseHeightSlope()
{
	//reverse 초기화
	bool bReverse = false;
	raiseDesc.SlopRev = 0;
	bool slopX = raiseDesc.SlopDir;

	int w = (int)(width + 1)*0.5f;
	int h = (int)(height + 1)*0.5f;
	//박스 만들기 위해 위치 조정
	raiseDesc.Box.x = (int)slopBox.x + w;
	raiseDesc.Box.y = (int)slopBox.z + w;
	raiseDesc.Box.z = -(int)slopBox.y + h;
	raiseDesc.Box.w = -(int)slopBox.w + h;

	if (raiseDesc.Box.x > raiseDesc.Box.y)
	{
		swap(raiseDesc.Box.x, raiseDesc.Box.y);
		if (slopX==true)
			bReverse = true;
	}
	if (raiseDesc.Box.z > raiseDesc.Box.w)
	{
		swap(raiseDesc.Box.z, raiseDesc.Box.w);
		if (slopX==false)
			bReverse = true;
	}

	if (raiseDesc.Box.x < 0) raiseDesc.Box.x = 0;
	if (raiseDesc.Box.y >= w * 2.0f) raiseDesc.Box.y = w * 2.0f;
	if (raiseDesc.Box.z < 0) raiseDesc.Box.z = 0;
	if (raiseDesc.Box.w >= h * 2.0f) raiseDesc.Box.w = h * 2.0f;
	if (bReverse) raiseDesc.SlopRev = 1;
	
	UpdateAlphaMap(2);
}

void TerrainLod::HeightNoise()
{
	if (perlinGen->CanUsing() == false)
		return;
	UpdateAlphaMap(1, 2);
}


void TerrainLod::HeightSmoothing()
{
	UpdateAlphaMap(0, 2);
}

void TerrainLod::UpdateAlphaMap(UINT pass, UINT tech)
{
	raiseBuffer->Apply();
	sRaiseBuffer->SetConstantBuffer(raiseBuffer->Buffer());
	
	raiseCS->AsSRV("HeightMap")->SetResource(HMapSrv);
	raiseCS->AsUAV("OutputMap")->SetUnorderedAccessView(raiseCT[0]->UAV());
	raiseCS->Dispatch(tech, pass, (int)(AlphaMapPixel.size()) / 1024, 1, 1);

	raiseCS->AsSRV("HeightMap2")->SetResource(raiseCT[0]->SRV());
	raiseCS->AsUAV("OutputMap2")->SetUnorderedAccessView(raiseCT[1]->UAV());
	raiseCS->Dispatch(3, 2, (int)(AlphaMapPixel.size()) / 1024, 1, 1);
	HMapSrv = raiseCT[1]->SRV();
	
	sAlphaTexture->SetResource(HMapSrv);


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
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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


	for (int z = raiseDesc.Box.z; z < raiseDesc.Box.w; z++)
	{
		for (int x = raiseDesc.Box.x; x < raiseDesc.Box.y; x++)
		{
			UINT index = desc.Width * z + x;

			CONST FLOAT f = 1.0f / 255.0f;

			float r = (float)((0xFF000000 & colors[index]) >> 24);
			float g = (float)((0x00FF0000 & colors[index]) >> 16);
			float b = (float)((0x0000FF00 & colors[index]) >> 8);
			float a = (float)((0x000000FF & colors[index]) >> 0);

			AlphaMapPixel[index] = (D3DXCOLOR(a, b, g, r)*f);
		}
	}

	SafeDeleteArray(colors);
	SafeRelease(texture);
}

void TerrainLod::CheckQuadCollider(QuadTreeNode* node, Collider* collider, vector< QuadTreeNode*>& updateNode)
{
	node->GetCollider()->Update();
	if (node->GetCollider()->IsIntersect(collider))
	{
		if (node->HasChilds())
		{
			for (QuadTreeNode* child : node->GetChildren())
			{
				CheckQuadCollider(child, collider, updateNode);
			}
		}
		else
			updateNode.emplace_back(node);
	}

}

void TerrainLod::UpdateQuadHeight()
{
	
	float TileSize = bufferDesc.TexScale * 2;

	int w = (int)(width)*0.5f;
	int h = (int)(height)*0.5f;
	Vector3 minV3(BrushedArea.x - w - TileSize, -100, -BrushedArea.w + h - TileSize);
	Vector3 maxV3(BrushedArea.y - w + TileSize, +100, -BrushedArea.z + h + TileSize);
	if (maxV3.x <= minV3.x
		|| maxV3.z <= minV3.z)
		return;

	CalcBoundY();
	Vector3 scale = maxV3 - minV3;
	Vector3 position = (maxV3 + minV3)*0.5f;
	Transform* transform = new Transform();
	transform->Position(position);
	transform->Scale(scale);
	AreaCol->ChangeTrans(transform);
	AreaCol->Update();

	vector< QuadTreeNode*> temp4updateNode;
	CheckQuadCollider(quadTree->RootNode, AreaCol, temp4updateNode);

	float cellF = 1.0f / info.CellSpacing;
	for (QuadTreeNode* node : temp4updateNode)
	{
		Collider* col = node->GetCollider();
		Vector2 TopLeft, BottomRight;
		//생성 역순으로 되찾기
		TopLeft = Vector2(col->GetMinRound().x+w, -col->GetMaxRound().z+h)*cellF;
		BottomRight = Vector2(col->GetMaxRound().x+w, -col->GetMinRound().z+h)*cellF;
		
		Vector2 minMaxY = GetMinMaxY(TopLeft, BottomRight);
		minMaxY.x = min(minMaxY.x, 0);
		minMaxY.x -= 0.05f;
		minMaxY.y += 0.05f;
		minMaxY *= bufferDesc.HeightRatio;
		Vector3 pos, scale;
		col->GetTransform()->Position(&pos);
		col->GetTransform()->Scale(&scale);
		scale.y = minMaxY.y - minMaxY.x;
		pos.y = (minMaxY.x + minMaxY.y)*0.5f;

		col->GetTransform()->Position(pos);
		col->GetTransform()->Scale(scale);		
	}
}

#pragma endregion

////////////////////////////////////
// QuadTree
////////////////////////////////////
#pragma region QuadTree

Vector2 TerrainLod::GetMinMaxY(Vector2& TopLeft, Vector2& BottomRight)
{
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;

	for (UINT y = TopLeft.y; y < BottomRight.y; y++)
	{
		for (UINT x = TopLeft.x; x < BottomRight.x; x++)
		{
			float data = 0.0f;
			if (InBounds(y, x))
			{
				data = AlphaMapPixel[y * (width+1) + x].a;
			}

			minY = min(minY, data);
			maxY = max(maxY, data);			
		}
	}
	return Vector2(minY, maxY);
}

QuadTreeNode* TerrainLod::CreateQuadTreeData(QuadTreeNode* parent,Vector2& TopLeft, Vector2& BottomRight)
{
	const float tolerance = 0.01f;
	Vector2 minMaxY = GetMinMaxY(TopLeft, BottomRight);
	minMaxY.x -= 0.05f;
	minMaxY.y += 0.05f;
	minMaxY *= bufferDesc.HeightRatio;
	float minX = TopLeft.x * info.CellSpacing - width *0.5f;
	float maxX = BottomRight.x * info.CellSpacing - width * 0.5f;
	float minZ = -TopLeft.y * info.CellSpacing + height *0.5f;
	float maxZ = -BottomRight.y * info.CellSpacing + height * 0.5f;
	
	// adjust the bounds to get a very slight overlap of the bounding boxes
	minX -= tolerance;
	maxX += tolerance;
	minZ += tolerance;
	maxZ -= tolerance;
	QuadTreeNode* quadNode = new QuadTreeNode();
	if (parent != NULL)
		quadNode->SetParent(parent);
	minMaxY.x = min(minMaxY.x, 0);

	Vector3 minV3(minX, minMaxY.x, maxZ);
	Vector3 maxV3(maxX, minMaxY.y, minZ);
	Vector3 scale=maxV3-minV3;
	Vector3 position = (maxV3 + minV3)*0.5f;
	Transform* transform = new Transform();
	transform->Position(position);
	transform->Scale(scale);
	Collider* collider = new Collider(transform);

	quadNode->SetCollider(collider);
	float narrow = (BottomRight.x - TopLeft.x)*0.5f;
	float depth  = (BottomRight.y - TopLeft.y) *0.5f;
	

	float TileSize = bufferDesc.TexScale*2;
	if (narrow >= TileSize && depth >= TileSize) {
		quadNode->AddChild(	CreateQuadTreeData(quadNode,TopLeft, Vector2(TopLeft.x + narrow, TopLeft.y + depth))		);
		quadNode->AddChild(	CreateQuadTreeData(quadNode,Vector2(TopLeft.x + narrow, TopLeft.y), Vector2(BottomRight.x, TopLeft.y + depth))		);
		quadNode->AddChild(	CreateQuadTreeData(quadNode,Vector2(TopLeft.x, TopLeft.y + depth), Vector2(TopLeft.x + depth, BottomRight.y))		); 
		quadNode->AddChild(	CreateQuadTreeData(quadNode,Vector2(TopLeft.x + narrow, TopLeft.y + depth), BottomRight)		); 
	}	
	return quadNode;
}

#pragma endregion


#pragma region ETC

void TerrainLod::TerrainController()
{
	bool bDocking = true;
	ImGui::Begin("Terrain_Controller", &bDocking);
	{
		/*Brush*/
		ImGui::Text("BrushPosition : %.2f,%.2f,%.2f", brushPos.x, brushPos.y, brushPos.z);

		static bool bLod = false;
		static bool bWire = false;
		ImGui::Checkbox("QuadFrame", &bQuadFrame);
		ImGui::SameLine();
		ImGui::Checkbox("LOD", &bLod);
		ImGui::Checkbox("WireFrame", &bWire);
		Pass(bWire ? 1 : 0);
		ImGui::SliderFloat("HeightRatio", &bufferDesc.HeightRatio, 10.0f, 100.0f);
		if (ImGui::Button("Apply Height"))
		{
			ID3D11Texture2D* srcTexture;
			HMapSrv->GetResource((ID3D11Resource **)&srcTexture);
			Texture::ReadPixels(srcTexture, DXGI_FORMAT_R8G8B8A8_UNORM, &AlphaMapPixel);

			SafeDelete(quadTree);
			QuadTreeNode* root = CreateQuadTreeData(NULL, Vector2(0, 0), Vector2(width, height));
			quadTree = new QuadTree(root);
		}
		shader->AsScalar("UseLOD")->SetInt(bLod ? 1 : 0);
		ImGui::Separator();
		ImGui::ImageButton(HMapSrv, ImVec2(120, 120));
		{
			if (ImGui::Button("SaveXml", ImVec2(60, 25)))
			{
				SaveTerrainToXml();
			}
			ImGui::SameLine(80);
			if (ImGui::Button("LoadXml", ImVec2(60, 25)))
			{
				LoadTerrainFromXml();
			}
		}
		ImGui::Separator();
		///////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("LineParts"))
		{
			ImGui::ColorEdit3("LineColor", (float*)& lineColorDesc.Color);
			ImGui::SliderInt("VisibleLine", (int*)&lineColorDesc.Visible, 0, 1);
			int size = lineColorDesc.Size;
			int max = info.CellsPerPatch*2;
			int i = 0;
			while (max > 1)
			{
				max=max >> 1;
				i++;
			}
			max = i;
			i = 0;
			while (size > 1)
			{
				size=size >> 1;
				i++;
			}
			size = i;
			ImGui::SliderInt("LineSize(Powered)", &size, 0, max);
			lineColorDesc.Size = 1<<(size);
			ImGui::Text("LineSize : %d", (int)lineColorDesc.Size);
			ImGui::SliderFloat("LineThickness", &lineColorDesc.Thickness, 0.01f, 1.0f);
		}
		ImGui::Separator();
		///////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("BrushParts", ImGuiTreeNodeFlags_DefaultOpen))
		{
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
						brushDesc.Type = n;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			raiseDesc.BrushType = brushDesc.Type;


			if (brushDesc.Type == 0)
			{
			}
			else if (brushDesc.Type < 3)
			{
				ImGui::ColorEdit3("BrushColor", (float*)& brushDesc.Color);
				bool bRangeChanged=ImGui::InputInt("BrushRange", (int*)& brushDesc.Range);
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

					
					if (raiseDesc.RaiseType == 3 && bRangeChanged == true)
					{
						perlinGen->Resize((UINT)raiseDesc.Radius*2);
						raiseCS->AsSRV("PerlinMap")->SetResource(perlinGen->GetPerlinSrv());
					}
					

					ImGui::SliderFloat("RaiseRate", &raiseRate, 10.0f, 300.0f);
					raiseDesc.Rate = raiseRate * Time::Delta();

					if (brushDesc.Type == 2)
					{
						ImGui::SliderFloat("BrushFactor", &rfactor, 0.1f, 10.0f);
						raiseDesc.Factor = rfactor;
					}
				}
				else if (bSplat)
				{
					ImGui::SliderFloat("BrushGrad", &gfactor, 0.0f, 100.0f);
					raiseDesc.Factor = gfactor;
				}
			}
			else if (brushDesc.Type == 3)
			{
				bool bSlopX = raiseDesc.SlopDir == 1 ? true : false;

				ImGui::Checkbox("SlopX", &bSlopX);
				raiseDesc.SlopDir = bSlopX ? 1 : 0;
				ImGui::SliderFloat("SlopeAngle", &sAngle, 0.0f, Math::ToRadian(80));
				raiseDesc.Factor = sAngle;
			}
		}

		ImGui::Separator();
		///////////////////////////////////////////////////////////////////////////
		// Splatting Layer
		if (ImGui::CollapsingHeader("Layers"))
		{
			ImGui::Checkbox("Splatting", &bSplat);
			ID3D11ShaderResourceView* srv = { 0 };
			if (alphaTexture != NULL)
				srv = alphaTexture->SRV();
			ImGui::PushID("AlphaTexture");
			if (ImGui::ImageButton(srv, ImVec2(50, 50)))
			{
				OpenTextureLayer(TextureLayerType::Alpha);
			}
			ImGui::PopID();
			ImGui::SameLine(100);
			ImGui::Text("AlphaTexture"); 

			//위쪽하고 같이 널값이면 id 같아서 실행 안됨;
			srv = NULL;
			ImGui::PushID("BaseTexture");
			if (baseTexture != NULL)
				srv = baseTexture->SRV();
			if (ImGui::ImageButton(srv, ImVec2(50, 50)))
			{
				OpenTextureLayer(TextureLayerType::Base);
			}
			ImGui::PopID();
			ImGui::SameLine(100);
			ImGui::Text("BaseTexture");
			/*if (normalTexture != NULL)
				srv = normalTexture->SRV();
			if (ImGui::ImageButton(srv, ImVec2(50, 50)))
			{
			}
			ImGui::SameLine(100);
			ImGui::Text("BaseNormalTexture");*/
			for (int i = 0; i < 3; i++)
			{
				ID3D11ShaderResourceView* srv = { 0 };
				string label = "Layer" + to_string(i + 1) + "Texture";
				ImGui::PushID(label.c_str());

				if (layerTexture[i] != NULL)
					srv = layerTexture[i]->SRV();
				if (ImGui::ImageButton(srv, ImVec2(50, 50)))
				{
					OpenTextureLayer(TextureLayerType(TextureLayerType::Layer_1 + i));
				}
				ImGui::PopID();
				ImGui::SameLine(100);
				ImGui::RadioButton(label.c_str(), &raiseDesc.SplattingLayer, i);
			}
		}
		///////////////////////////////////////////////////////////////////////////
	}
	
	ImGui::End();

	if (raiseDesc.RaiseType == 3)
	{
		perlinGen->PerlinController();
		PerlinPixel = perlinGen->GetPixels();
	}
}

void TerrainLod::OpenTextureLayer(TextureLayerType type, const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::ImageFilter, L"", bind(&TerrainLod::OpenTextureLayer, this, type, placeholders::_1));
	}
	else
	{
		switch (type)
		{
		case TextureLayerType::Base:
			BaseTexture(filePath);
			break;
		case TextureLayerType::Alpha:
			{
				int result = MessageBox(
					NULL,
					L"Alpha를 사용하도록 변환합니다.",
					L"AlphaMap 변환여부",
					MB_OKCANCEL
				);
				if (result == IDOK)
					AlphaTexture(filePath,true);
				else if (result == IDCANCEL)
					AlphaTexture(filePath);
			}
			break;
		case TextureLayerType::Layer_1:
			LayerTextures(filePath, 0);
			break;
		case TextureLayerType::Layer_2:
			LayerTextures(filePath, 1);
			break;
		case TextureLayerType::Layer_3:
			LayerTextures(filePath, 2);
			break;
		default:
			break;
		}
	}
}

void TerrainLod::SaveAlphaLayer(wstring* savePath, const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::SaveFileDialog(L"", Path::ImageFilter, L"", bind(&TerrainLod::SaveAlphaLayer, this, savePath,placeholders::_1));
	}
	else
	{
		wstring oPath = filePath;

		//png 확장자로 확정하기
		if (Path::GetExtension(oPath).compare(L"png") != 0)
			oPath =
			Path::GetDirectoryName(oPath)
			+Path::GetFileNameWithoutExtension(oPath) + L".png";

		// 알파맵 저장 구간
		ID3D11Texture2D* srcTexture;
		HMapSrv->GetResource((ID3D11Resource **)&srcTexture);

		D3D11_TEXTURE2D_DESC srcDesc;
		srcTexture->GetDesc(&srcDesc);

		ID3D11Texture2D* dstTexture;

		D3D11_TEXTURE2D_DESC dstDesc;
		ZeroMemory(&dstDesc, sizeof(D3D11_TEXTURE2D_DESC));
		srcTexture->GetDesc(&dstDesc);
		dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		dstDesc.Usage = D3D11_USAGE_STAGING;
		dstDesc.BindFlags = 0;
		dstDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; 얘로 바뀌길래 포멧 고정
		//
		Check(D3D::GetDevice()->CreateTexture2D(&dstDesc, NULL, &dstTexture));

		D3D::GetDC()->CopyResource(dstTexture, srcTexture);


		D3D11_MAPPED_SUBRESOURCE map;
		D3D::GetDC()->Map(dstTexture, 0, D3D11_MAP_READ, NULL, &map);
		{
			UINT* colors = (UINT*)map.pData;

			for (UINT y = 0; y < srcDesc.Height; y++)
			{
				for (UINT x = 0; x < srcDesc.Width; x++)
				{
					//이미지 크기에 따른 인덱스 설정
					UINT index = map.RowPitch * y / 4 + x;
					//픽셀 뒤집어서 뽑기
					UINT pixel = srcDesc.Width * (y) + x;
					Color temp;

					temp.r = AlphaMapPixel[pixel].r;
					temp.g = AlphaMapPixel[pixel].g;
					temp.b = AlphaMapPixel[pixel].b;
					temp.a = AlphaMapPixel[pixel].a;

					colors[index] = (UINT)(temp.r * 255.0f) | (UINT)(temp.g * 255.0f) << 8 | (UINT)(temp.b * 255.0f) << 16 | (UINT)(temp.a*255.0f) << 24;
				}
			}
		}
		D3D::GetDC()->Unmap(dstTexture, 0);

		Texture::SaveFile(oPath, dstTexture);
		*savePath = oPath;
	}
}

void TerrainLod::SaveTerrainToXml(const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::SaveFileDialog(L"", Path::XmlFilter, L"", bind(&TerrainLod::SaveTerrainToXml, this, placeholders::_1));
	}
	else
	{
		string folder = String::ToString(Path::GetDirectoryName(filePath));
		string file = String::ToString(Path::GetFileName(filePath));

		Path::CreateFolders(folder);

		Xml::XMLDocument* document = new Xml::XMLDocument();

		Xml::XMLDeclaration* decl = document->NewDeclaration();
		document->LinkEndChild(decl);

		Xml::XMLElement* root = document->NewElement("TerrainInfo");
		document->LinkEndChild(root);

		Xml::XMLElement* alpha = document->NewElement("AlphaTexture");
		
		wstring alphaPath;
		SaveAlphaLayer(&alphaPath);

		if (alphaTexture != NULL)
			alpha->SetAttribute("Path",String::ToString(alphaPath).c_str());
		else
			alpha->SetAttribute("Path", "NoTex");
		root->LinkEndChild(alpha);

		Xml::XMLElement* base = document->NewElement("BaseTexture");
		if(baseTexture!=NULL)
			base->SetAttribute("Path", String::ToString(baseTexture->GetFile()).c_str());
		else
			base->SetAttribute("Path", "NoTex");
		root->LinkEndChild(base);

		Xml::XMLElement* layers = document->NewElement("LayerTextures");
		root->LinkEndChild(layers);
		for (int i = 0; i < 3; i++)
		{
			Xml::XMLElement* element = NULL;
			string name = "Layer_0" + to_string(i + 1);
			element = document->NewElement(name.c_str());
			if (layerTexture[i] != NULL)
				element->SetAttribute("Path", String::ToString(layerTexture[i]->GetFile()).c_str());
			else
				element->SetAttribute("Path", "NoTex");
			layers->LinkEndChild(element);
		}

		if (Path::GetExtension(file).compare("xml") != 0)
			file = file + ".xml";

		document->SaveFile((folder + file).c_str());
	}
}

void TerrainLod::LoadTerrainFromXml(const wstring & filePath)
{
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::XmlFilter, L"", bind(&TerrainLod::LoadTerrainFromXml, this, placeholders::_1));
	}
	else
	{
		string base, alpha, layer[3];

		Xml::XMLDocument* document = new Xml::XMLDocument();
		Xml::XMLError error = document->LoadFile(String::ToString(filePath).c_str());
		assert(error == Xml::XML_SUCCESS);

		Xml::XMLElement* root = document->FirstChildElement();
		Xml::XMLElement* layerNode = root->FirstChildElement();

		alpha = layerNode->Attribute("Path");	
		layerNode = layerNode->NextSiblingElement();
		AlphaTexture(String::ToWString(alpha));

		base = layerNode->Attribute("Path");		
		BaseTexture(String::ToWString(base));
		layerNode = layerNode->NextSiblingElement();


		Xml::XMLElement* node = NULL;
		node = layerNode->FirstChildElement();
		for (int i = 0; i < 3; i++)
		{

			layer[i] = node->Attribute("Path");

			if (layer[i].find("NoTex") != string::npos)
				continue;
			{
				LayerTextures(String::ToWString(layer[i]),i);
			}
			node = node->NextSiblingElement();
		}
	}
}

#pragma endregion
