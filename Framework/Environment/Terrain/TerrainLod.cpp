#include "Framework.h"
#include "TerrainLod.h"
#include "Viewer/Fixity.h"
#include "Utilities/Perlin.h"
#include "Utilities/Xml.h"


TerrainLod::TerrainLod(InitializeInfo& info)
	:Renderer(info.shader), baseTexture(NULL)
	//, BrushedArea(+FLT_MAX, -0, +FLT_MAX, -0)
{
	this->info = info;
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);


	//////////////////////////////////////////////////////////////////////////////
	// Const buffer & shader effects
	//////////////////////////////////////////////////////////////////////////////

	sBaseTexture = shader->AsSRV("BaseMap");
	sLayerTexture = shader->AsSRV("LayerMaps");
	sNormalTexture = shader->AsSRV("NormalMap");
	sAlphaTexture = shader->AsSRV("AlphaMap");

	uvPickShader = SETSHADER(L"HW08_TerrainLoDBrush.fx");
	computeBuffer = new StructuredBuffer
	(
		NULL,
		sizeof(Color), 1,
		true
	);

	texelBuffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sTexelBuffer = shader->AsConstantBuffer("CB_Terrain");
	
	CreateInitHeightMap();
	
	Initialize();

	AreaCol = new OBBCollider();
	AreaCol->AddInstance();
	AreaCol->SetDebugMode(true);

	float width = D3D::Width();
	float height = D3D::Height();
	renderTarget = new RenderTarget((UINT)width, (UINT)height);
	depthStencil = new DepthStencil(width, height);
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
}


void TerrainLod::CreateInitHeightMap()
{
	ID3D11Texture2D* texture;

	//CreateTexture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = 512;
		desc.Height = 512;
		desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		
		Check(D3D::GetDevice()->CreateTexture2D(&desc, NULL, &texture));
	}

	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &HMapSrv));
	}
}

void TerrainLod::Initialize()
{
	////////////////////////////////////////
	// 알파맵 사이즈 변화에 따라 바뀌어야할것들
	{
		ID3D11Texture2D* srcTexture;
		HMapSrv->GetResource((ID3D11Resource **)&srcTexture);
		D3D11_TEXTURE2D_DESC desc;
		srcTexture->GetDesc(&desc);
		width = desc.Width - 1;
		height = desc.Height - 1;
		Texture::ReadPixels(srcTexture, DXGI_FORMAT_R16G16B16A16_UNORM, &AlphaMapPixel);
	}
	bufferDesc.TexelCellSpaceU = 1.0f / ((float)width);
	bufferDesc.TexelCellSpaceV = 1.0f / ((float)height);
	bufferDesc.HeightRatio = info.HeightRatio;
	bufferDesc.TexScale = info.CellSpacing*2.0f;

	patchVertexRows = (width / info.CellsPerPatch) + 1;
	patchVertexCols = (height / info.CellsPerPatch) + 1;

	vertexCount = patchVertexRows * patchVertexCols;
	faceCount = (patchVertexRows - 1) * (patchVertexCols - 1);
	indexCount = faceCount * 4;

	CalcBoundY();
	CreateVertexData();
	CreateIndexData();

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTerrain));
	indexBuffer = new IndexBuffer(indices, indexCount);

	quadTree = new QuadTree();
	QuadTreeNode* root = CreateQuadTreeData(NULL, Vector2(0, 0), Vector2(width, height));
	quadTree->SetRootNode(root);
	quadTree->Update();
}

void TerrainLod::PreRender()
{
	if (Mouse::Get()->Press(0))return;
	//Terrain UV Picking을 위한 프리랜더
	// UV값을 r,g에 받은 형태로 랜더할거임.
	renderTarget->Set(depthStencil->DSV());
	{
		Super::Render();
		texelBuffer->Apply();
		sTexelBuffer->SetConstantBuffer(texelBuffer->Buffer());
		shader->DrawIndexed(0, 2, indexCount);
	}
	preTerrainSrv = renderTarget->SRV();
	//render2D->SRV(preTerrainSrv);
}

void TerrainLod::Update()
{	
	Super::Update();
	   	 
}

void TerrainLod::Render()
{
	Super::Render();
	
	
	if (baseTexture != NULL)
		sBaseTexture->SetResource(baseTexture->SRV());
	if (normalTexture != NULL)
		sNormalTexture->SetResource(normalTexture->SRV());

	sLayerTexture->SetResourceArray(layerViews,0,3);
		
	texelBuffer->Apply();
	sTexelBuffer->SetConstantBuffer(texelBuffer->Buffer());
	shader->DrawIndexed(Tech() , Pass(), indexCount);

	//TODO: 나중 삭제
	//TestCol->Render(Color(1, 1, 0, 1));
	//render2D->Render();
}

void TerrainLod::QuadTreeRender()
{
	quadTree->Render();
}

#pragma region Texture
////////////////////////////////////
// Textures
////////////////////////////////////

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
	
	bChangeAlpha = true;
	this->bUseAlpha = bUseAlpha;
	
	Initialize();
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

#pragma endregion

////////////////////////////////////
// Lod Creaters
////////////////////////////////////
#pragma region Lod 생성관련


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


#pragma endregion

#pragma region Pick

////////////////////////////////////
// Pick
////////////////////////////////////

Vector3 TerrainLod::GetPickedPosition()
{
	/*
	Terrain UV Picking
		1. 쉐이더에 마우스 좌표와 프리랜더로 랜더한 터레인의 uv 랜더타겟을 넘긴다.
		2. 마우스좌표에 위치한 픽셀값을 읽는다.
		3. 받아온 Color값에 보정값을 계산하고 uv와 터레인의 높이좌표값이 반대이므로 뒤집는다.
	*/
	Vector3 mouse = Mouse::Get()->GetPosition();
	//브러시 좌표의 시작점을 계산하기 위함
	Vector3 result = Vector3(-0.5f*width,0.0f, -0.5f*height);
	//터레인 내부에서 따로 CS 만들어서 적용해주기.
	uvPickShader->AsVector("MousePos")->SetFloatVector((float*)&mouse);
	uvPickShader->AsSRV("Terrain")->SetResource(preTerrainSrv);
	uvPickShader->AsUAV("OutputPickColor")->SetUnorderedAccessView(computeBuffer->UAV());
	uvPickShader->Dispatch(3, 2, 1, 1, 1);
	
	computeBuffer->Copy(PickColor, sizeof(Color));
	result.x += PickColor.r*width;
	result.z += PickColor.g*height;
	result.z = -result.z;
	
	result.y = GetPickedHeight(result);

	return result;
}

float TerrainLod::GetPickedHeight(const Vector3& position)
{
	int w = (int)(width)*0.5f;
	int h = (int)(height)*0.5f;
	int x = (int)ceil(position.x) + w;
	int z = -(int)ceil(position.z) + h;

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
	p[0] = Vector3(x, AlphaMapPixel[index[0]].a, z);
	p[1] = Vector3(x, AlphaMapPixel[index[1]].a, z+1);
	p[2] = Vector3(x+1, AlphaMapPixel[index[2]].a, z);
	p[3] = Vector3(x+1, AlphaMapPixel[index[3]].a, z+1);
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
	if (result.y > 0)
		result.y *= bufferDesc.HeightRatio;
	return result.y;
}

#pragma endregion

#pragma region QuadTree

void TerrainLod::CheckQuadCollider(QuadTreeNode* node, vector< QuadTreeNode*>& updateNode)
{
	if (node->IsIntersect())
	{
		if (node->HasChilds())
		{
			//updateNode.emplace_back(node);

			for (QuadTreeNode* child : node->GetChildren())
			{
				CheckQuadCollider(child, updateNode);
			}
		}
		else
			updateNode.emplace_back(node);
	}

}

void TerrainLod::UpdateQuadHeight(const Vector4& BrushedArea)
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

	AreaCol->GetTransform()->Position(position);
	AreaCol->GetTransform()->Scale(scale);
	AreaCol->Update();

	vector< QuadTreeNode*> temp4updateNode;
	quadTree->Update();
	OBBCollider* col = quadTree->GetCollider();

	//OBB끼리 충돌을 T0,P1로 해놨음
	// 충돌 계산 직후에 값을 사용할것
	quadTree->GetCollider()->ComputeColliderTest(0, 0, AreaCol);
	CheckQuadCollider(quadTree->GetRootNode(), temp4updateNode);

	float cellF = 1.0f / info.CellSpacing;
	for (QuadTreeNode* node : temp4updateNode)
	{
		//OBBCollider* col = node->GetCollider();
		UINT inst = node->GetColInst();
		Vector2 TopLeft, BottomRight;

		Vector3 MinR = col->GetMinRound(inst);
		Vector3 MaxR = col->GetMaxRound(inst);

		//생성 역순으로 되찾기
		TopLeft = Vector2(MinR.x + w, -MaxR.z + h)*cellF;
		BottomRight = Vector2(MaxR.x + w, -MinR.z + h)*cellF;

		Vector2 minMaxY = GetMinMaxY(TopLeft, BottomRight);
		minMaxY.x = min(minMaxY.x, 0);
		minMaxY.y = max(minMaxY.y, 0);
		minMaxY *= bufferDesc.HeightRatio;
		minMaxY.x -= 0.05f;
		minMaxY.y += 0.05f;
		Vector3 pos, scale;
		col->GetTransform(inst)->Position(&pos);
		col->GetTransform(inst)->Scale(&scale);
		scale.y = minMaxY.y - minMaxY.x;
		pos.y = (minMaxY.x + minMaxY.y)*0.5f;

		col->GetTransform(inst)->Position(pos);
		col->GetTransform(inst)->Scale(scale);
	}
	quadTree->Update();
}

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

QuadTreeNode* TerrainLod::CreateQuadTreeData(QuadTreeNode* parent, Vector2& TopLeft, Vector2& BottomRight)
{
	const float tolerance = 0.01f;
	Vector2 minMaxY = GetMinMaxY(TopLeft, BottomRight);
	minMaxY *= bufferDesc.HeightRatio;
	minMaxY.x = min(minMaxY.x, -1.0f);
	minMaxY.y = max(minMaxY.y, 1.0f);
	minMaxY.x -= 0.05f;
	minMaxY.y += 0.05f;
	float minX = TopLeft.x * info.CellSpacing - width * 0.5f;
	float maxX = BottomRight.x * info.CellSpacing - width * 0.5f;
	float minZ = -TopLeft.y * info.CellSpacing + height * 0.5f;
	float maxZ = -BottomRight.y * info.CellSpacing + height * 0.5f;

	// adjust the bounds to get a very slight overlap of the bounding boxes
	minX -= tolerance;
	maxX += tolerance;
	minZ += tolerance;
	maxZ -= tolerance;
	QuadTreeNode* quadNode = new QuadTreeNode(quadTree);
	if (parent != NULL)
		quadNode->SetParent(parent);
	minMaxY.x = min(minMaxY.x, 0);

	Vector3 minV3(minX, minMaxY.x, maxZ);
	Vector3 maxV3(maxX, minMaxY.y, minZ);
	Vector3 scale = maxV3 - minV3;
	Vector3 position = (maxV3 + minV3)*0.5f;
	Transform* transform = new Transform();
	transform->Position(position);
	transform->Scale(scale);
	//인스턴스번호
	UINT colinst = quadNode->GetColInst();
	//인스턴스추가
	quadTree->GetCollider()->AddInstance(transform);


	float narrow = (BottomRight.x - TopLeft.x)*0.5f;
	float depth = (BottomRight.y - TopLeft.y) *0.5f;

	float TileSize = info.CellsPerPatch;
	if (narrow >= TileSize && depth >= TileSize) {
		//자식쪽만 키게하려고
		//quadTree->GetCollider()->SetdCollisionOff(colinst);
		quadNode->AddChild(CreateQuadTreeData(quadNode, TopLeft, Vector2(TopLeft.x + narrow, TopLeft.y + depth)));
		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x + narrow, TopLeft.y), Vector2(BottomRight.x, TopLeft.y + depth)));
		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x, TopLeft.y + depth), Vector2(TopLeft.x + depth, BottomRight.y)));
		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x + narrow, TopLeft.y + depth), BottomRight));
	}
	return quadNode;
}

#pragma endregion


#pragma region ETC

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
		//저장은 16짜리로 할 필요 없음.
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

void TerrainLod::TerrainLayerProperty(bool* bChangeAlpha, bool* bUseAlpha)
{
	///////////////////////////////////////////////////////////////////////////
		// Splatting Layer
	if (ImGui::CollapsingHeader("Layers"))
	{
		ID3D11ShaderResourceView* srv = { 0 };

		//알파맵은 기본생성을 함.
		srv = HMapSrv;

		ImGui::PushID("AlphaTexture");
		if (ImGui::ImageButton(srv, ImVec2(50, 50)))
		{
			OpenTextureLayer(TextureLayerType::Alpha);
			*bChangeAlpha = this->bChangeAlpha;
			*bUseAlpha = this->bUseAlpha;
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
			//ImGui::SameLine(100);
		}
	}
}


#pragma endregion
