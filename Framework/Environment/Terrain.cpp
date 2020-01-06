#include "Framework.h"
#include "Terrain.h"


Terrain::Terrain(Shader* shader, wstring heightMap)
	:Renderer(shader), baseMap(NULL), spacing(3, 3)	//텍스쳐를 3장 타일링 하겠다?
	, alphaMap(NULL), layerMap(NULL), layerIndex(0)
	, minHeight(3000.0f)
{
	this->heightMap = new Texture(heightMap);
	sAlphaMap = shader->AsSRV("AlphaMap");
	AlphaMap(heightMap);
	for (int i = 0; i < 3; i++)
		layerMaps[i] = NULL;

	CreateVertexData();
	CreateIndexData();
	CreateNormalData();		//정점하고 인덱스를 가져다 쓰니까 순서가 이래야함

	sBaseMap = shader->AsSRV("BaseMap");

	//CreateVertexBuffer
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(TerrainVertex), 0, true);
	//CreateIndexBuffer
	indexBuffer = new IndexBuffer(indices, indexCount);
	brushBuffer = new ConstantBuffer(&brushDesc, sizeof(BrushDesc));
	sBrushBuffer = shader->AsConstantBuffer("CB_TerrainBrush");

	lineColorBuffer = new ConstantBuffer(&lineColorDesc, sizeof(LineColorDesc));
	sLineColorBuffer = shader->AsConstantBuffer("CB_GridLine");


	sLayerMap = shader->AsSRV("LayerMap");
	sLayerMaps = shader->AsSRV("LayerMaps");

	/* CS용 shader*/
	CS_shader = new Shader(L"043_InterSection.fx");

}

Terrain::~Terrain()
{
	SafeDelete(brushBuffer);
	SafeDelete(heightMap);
	SafeDelete(baseMap);
	SafeDelete(alphaMap);
	for (int i = 0; i < 3; i++)
		SafeDelete(layerMaps[i]);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

}

void Terrain::Update()
{
	static bool bLineVisible = false;
	ImGui::Checkbox("LineVisible", &bLineVisible);
	if (bLineVisible)
		lineColorDesc.Visible = 1;
	else
		lineColorDesc.Visible = 0;
	ImGui::ColorEdit3("LineColor", (float*)& lineColorDesc.Color);
	ImGui::InputFloat("Thickness", &lineColorDesc.Thickness, 0.1f);
	ImGui::InputFloat("Size", &lineColorDesc.Size, 0.1f);
	
	
	ImGui::Separator();
	/*Brush*/
	ImGui::InputInt("BrushType", (int*)& brushDesc.Type);
	brushDesc.Type %= 3;

	static bool bSplat = false;
	ImGui::Checkbox("Splatting", &bSplat);
	//pickup과 달리 숫자로 바로 선택?
	ImGui::ColorEdit3("BrushColor", (float*)& brushDesc.Color);
	ImGui::InputInt("BrushRange", (int*)& brushDesc.Range);

	static float rfactor = 1.0f;
	static float gfactor = 1.0f;
	if (bSplat == false)
	{
		ImGui::SliderFloat("BrushFactor", &rfactor, 0.1f, 10.0f);
	}
	else if (bSplat)
	{
		ImGui::SliderFloat("BrushGrad", &gfactor, 0.0f, 100.0f);
	}
	ImGui::Separator();

	//레이즈 여부 초기값은 false로
	bool bRaise = false;
	//전역으로 지정 위치 설정, 초기값은 맵 밖인 -1로
	static Vector3 position(-1, -1, -1);

	if (brushDesc.Type > 0)
	{
		if (bSplat && Mouse::Get()->Press(0) && Keyboard::Get()->Press(VK_SHIFT))
		{
			if (brushDesc.Type == 2 && layerMaps[layerIndex] != NULL)
			{
				Splatting(position, brushDesc.Range, gfactor);
			}
		}

		if (bSplat == false && Mouse::Get()->Press(1) && Keyboard::Get()->Press(VK_SHIFT))
		{
			if (brushDesc.Type == 1)
				RaiseHeightQuad(position, brushDesc.Type, brushDesc.Range);
			if (brushDesc.Type == 2)
				RaiseHeightCircle(position, brushDesc.Type, brushDesc.Range, rfactor);
			bRaise = true;
		}

		if (bRaise == false)
		{
			position = GetPickedPosition();
			//터레인의 이동한 포지션 받기
			GetTransform()->Position(&brushDesc.Location);
			//마우스 위치 더하기
			brushDesc.Location += position;
		}
	}


	//같은것.
	//Renderer::Update();
	Super::Update();
	
}

void Terrain::Render()
{
	Super::Render();
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

	{
		sLayerMaps->SetResourceArray(layerViews, 0, 3);
	}

	shader->DrawIndexed(0, Pass(), indexCount);
}

void Terrain::BaseMap(wstring file)
{
	SafeDelete(baseMap);

	baseMap = new Texture(file);
	sBaseMap->SetResource(baseMap->SRV());
}

void Terrain::LayerMap(wstring layer, wstring alpha)
{
	SafeDelete(layerMap);
	SafeDelete(alphaMap);

	layerMap = new Texture(layer);
	alphaMap = new Texture(alpha);

	sLayerMap->SetResource(layerMap->SRV());
	sAlphaMap->SetResource(alphaMap->SRV());
}

void Terrain::LayerMaps(wstring layer, UINT layerIndex)
{
	this->layerIndex = layerIndex;
	SafeDelete(layerMaps[layerIndex]);
	layerMaps[layerIndex] = new Texture(layer);
	layerViews[layerIndex] = layerMaps[layerIndex]->SRV();

	sLayerMaps->SetResourceArray(layerViews, 0, 3);
}

void Terrain::AlphaMap(wstring alpha)
{
	SafeDelete(alphaMap);

	alphaMap = new Texture(alpha);

	sAlphaMap->SetResource(alphaMap->SRV());

	minHeight = 3000.0f;
	LoadAlphaMap();
}
float Terrain::GetHeight(Vector3 & position, Vector3& pUp)
{
	//플레이어 위치를 정수로 떨굼
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x<0 || x>=width) return -1.0f;	//x 범위 벗어남
	if (z<0 || z>=height) return -1.0f;	//z 범위 벗어남

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + x + 1;
	index[3] = width * (z + 1) + x + 1;

	Vector3 v[4], n[4];
	for (int i = 0; i < 4; i++)
	{
		if (index[i] >= vertexCount) return -1.0f;

		v[i] = vertices[index[i]].Position;
		n[i] = vertices[index[i]].Normal;
	}

	float ddx = (position.x - v[0].x) / 1.0f;
	float ddz = (position.z - v[0].z) / 1.0f;

	Vector3 temp, pForward, pRight;
	if (ddx + ddz <= 1)
	{
		temp = v[0] + (v[2] - v[0])*ddx + (v[1] - v[0])*ddz;
		//Cube가 있는 Plane의 Forward,Right 계산
		pForward = v[1] - v[0];
		pRight = v[2] - v[0];
	}
	else
	{
		ddx = 1 - ddx;
		ddz = 1 - ddz;
		temp = v[3] + (v[1] - v[3])*ddx + (v[2] - v[3])*ddz;

		pForward = v[3] - v[2];
		pRight = v[3] - v[1];
	}

	//Cube가 있는 Plane의 Up 계산
	D3DXVec3Cross(&pUp, &pForward, &pRight);
	//단위 벡터 화.
	D3DXVec3Normalize(&pUp, &pUp);

	return temp.y;
}

//격자 크기가 달라도 상관 없음.
float Terrain::GetPickedHeight(Vector3 & position)
{
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x<0 || x>=width) return -1.0f;	//x 범위 벗어남
	if (z<0 || z>=height) return -1.0f;	//z 범위 벗어남

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + x + 1;
	index[3] = width * (z + 1) + x + 1;

	Vector3 p[4];
	for (int i = 0; i < 4; i++)
	{
		if (index[i] >= vertexCount) return -1.0f;
		p[i] = vertices[index[i]].Position;
	}

	float u, v, distance;

	//빛의 시작점
	Vector3 start(position.x, 1000, position.z);
	//빛을 쏠 방향
	Vector3 direction(0, -1, 0);

	//충돌 결과, 없으면 초기값
	Vector3 result(-1, -1, -1);

	//raypos 반직선 위치
	//u,v 기울기
	//distance 충돌 지점으로 부터의 거리
	if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;

	if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;


	return result.y;
}

Vector3 Terrain::GetPickedPosition()
{
	//반직선의 시작점
	Vector3 start, direction;
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

	Vector3 mouse = Mouse::Get()->GetPosition();

	Matrix world = GetTransform()->World();

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


	/* CS에 값 넣기 */
	/* 레이 구조체 필요없이 있는거 사용 */
	CS_shader->AsVector("Position")->SetFloatVector(start);
	CS_shader->AsVector("Direction")->SetFloatVector(direction);
	CS_shader->AsSRV("Input")->SetResource(buffer->SRV());
	CS_shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	/* numthreads를 512 크기로 줬음, 기존 사용이 256*256 이라 충분히 크기 줌 */
	/* fx파일에서 GroupID.x를 가지고 주소를 찾는데 y,z는 따로 계산 안해서 x값만 크게 줌 */
	CS_shader->Dispatch(0, 0, 512, 1, 1);

	buffer->Copy(output, sizeof(OutputDesc) * size);


	for (UINT i = 0; i < size; i++)
	{
		OutputDesc temp = output[i];
		/* 피킹 되었으면 해당 위치 리턴*/
		if (temp.Picked == 1)
		{
			return input[i].V0 + (input[i].V1 - input[i].V0) * temp.U + (input[i].V2 - input[i].V0) * temp.V;
		}
	}

	/*
	//x,z 모르니 다 돌림
	//그래서 먼곳에 가면 느려짐
	//맨 끝 삼각형 만들 수 없어서 땡김
	for (UINT z = 0; z < height - 1; z++)
	{
		for (UINT x = 0; x < width - 1; x++)
		{
			UINT index[4];
			index[0] = width * z + x;
			index[1] = width * (z + 1) + x;
			index[2] = width * z + x + 1;
			index[3] = width * (z + 1) + x + 1;
			Vector3 p[4];
			for (int i = 0; i < 4; i++)
				p[i] = vertices[index[i]].Position;
			float u, v, distance;
			//distance 충돌 지점으로 부터의 거리
			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;
			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
				return p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
		}
	}
	*/
	return Vector3(-1, -1, -1);
}

void Terrain::RaiseHeightQuad(Vector3 & position, UINT type, UINT range)
{
	D3D11_BOX box;
	box.left = (UINT)position.x - range;
	box.top = (UINT)position.z + range;
	box.right = (UINT)position.x + range;
	box.bottom = (UINT)position.z - range;

	if (box.left < 0) box.left = 0;
	if (box.top >= height) box.top = height;
	if (box.right >= width) box.right = width;
	if (box.bottom < 0) box.bottom = 0;

	for (UINT z = box.bottom; z <= box.top; z++)
	{
		for (UINT x = box.left; x <= box.right; x++)
		{
			UINT index = width * z + x;
			vertices[index].Position.y += 5.0f * Time::Delta();
		}
	}
	CreateNormalData();

	//쓸려면 usage가 default여야
	//D3D::GetDC()->UpdateSubresource(vertexBuffer->Buffer(), 0, NULL, vertices, sizeof(TerrainVertex)*vertexCount, 0);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);


}

void Terrain::RaiseHeightCircle(Vector3 & position, UINT type, UINT range, float rfactor)
{
	D3D11_BOX box;
	box.left = (UINT)position.x - range;
	box.top = (UINT)position.z + range;
	box.right = (UINT)position.x + range;
	box.bottom = (UINT)position.z - range;

	if (box.left < 0) box.left = 0;
	if (box.top >= height) box.top = height;
	if (box.right >= width) box.right = width;
	if (box.bottom < 0) box.bottom = 0;

	for (UINT z = box.bottom; z <= box.top; z++)
	{
		for (UINT x = box.left; x <= box.right; x++)
		{
			UINT index = width * z + x;
			float dx = vertices[index].Position.x - position.x;
			float dz = vertices[index].Position.z - position.z;
			float dist = sqrt(dx * dx + dz * dz);
			if (dist <= range)
			{
				float angle = acosf(dist / range);
				float factor = sinf(angle) * rfactor;
				//(dist * radian > range ? 1.0f : dist * radian / range);
				vertices[index].Position.y += 5.0f * Time::Delta() * (factor);
			}
		}
	}
	CreateNormalData();

	//쓸려면 usage가 default여야
	//D3D::GetDC()->UpdateSubresource(vertexBuffer->Buffer(), 0, NULL, vertices, sizeof(TerrainVertex)*vertexCount, 0);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::Splatting(Vector3 & position, UINT range, float gfactor)
{
	D3D11_BOX box;
	box.left = (UINT)position.x - range;
	box.top = (UINT)position.z + range;
	box.right = (UINT)position.x + range;
	box.bottom = (UINT)position.z - range;

	if (box.left < 0) box.left = 0;
	if (box.top >= height) box.top = height;
	if (box.right >= width) box.right = width;
	if (box.bottom < 0) box.bottom = 0;

	for (UINT z = box.bottom; z <= box.top; z++)
	{
		for (UINT x = box.left; x <= box.right; x++)
		{
			UINT index = width * z + x;
			float dx = vertices[index].Position.x - position.x;
			float dz = vertices[index].Position.z - position.z;
			float dist = sqrt(dx * dx + dz * dz);
			if (dist <= range)
			{
				float factor = (1 - dist / range) * gfactor / 100.0f;

				switch (layerIndex)
				{
				case 0:
					vertices[index].Color.r += 1.0f * factor;
					vertices[index].Color.r = vertices[index].Color.r >= 1 ? 1.0f : vertices[index].Color.r;
					break;
				case 1:
					vertices[index].Color.g += 1.0f * factor;
					vertices[index].Color.g = vertices[index].Color.g >= 1 ? 1.0f : vertices[index].Color.g;
					break;
				case 2:
					vertices[index].Color.b += 1.0f * factor;
					vertices[index].Color.b = vertices[index].Color.b >= 1 ? 1.0f : vertices[index].Color.b;
					break;
				default:
					break;
				}
			}
		}
	}


	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

wstring Terrain::SaveTerrain()
{
	ID3D11Texture2D* srcTexture;
	alphaMap->SRV()->GetResource((ID3D11Resource * *)& srcTexture);

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

//	FILE* file = fopen("Savetest.csv", "w");

	D3D11_MAPPED_SUBRESOURCE map;
	D3D::GetDC()->Map(dstTexture, 0, D3D11_MAP_READ, NULL, &map);
	{
		//memcpy하면 에러 뜨길래 직접 대입
		UINT* colors = (UINT*)map.pData;

		for (UINT y = 0; y < srcDesc.Height; y++)
		{
			for (UINT x = 0; x < srcDesc.Width; x++)
			{
				//이미지 크기에 따른 인덱스 설정
				UINT index = map.RowPitch * y / 4 + x;
				UINT vIndex = srcDesc.Width * y + x;
				//픽셀 뒤집어서 뽑기
				UINT pixel = srcDesc.Width * (srcDesc.Height - 1 - y) + x;
				Color temp;

				temp.r = vertices[pixel].Color.r;
				temp.g = vertices[pixel].Color.g;
				temp.b = vertices[pixel].Color.b;
				//TODO : 배율 조정용도 받쟈..
				temp.a = (vertices[pixel].Position.y - minHeight);

				temp.a = temp.a <= 0.0f ? 0.0f : temp.a;
				temp.a = temp.a >= 255.0f ? 255.0f : temp.a;

				colors[index] = (UINT)(temp.r * 255.0f) | (UINT)(temp.g * 255.0f) << 8 | (UINT)(temp.b * 255.0f) << 16 | (UINT)(temp.a) << 24;
				//fprintf(file, "%d,%d,%d,%f,%f,%f,%f\n", index, pixel, (0x000000FF & colors[index]), temp.r, temp.g, temp.b, temp.a);
			}
		}
	}
	D3D::GetDC()->Unmap(dstTexture, 0);
	//fclose(file);

	wstring saveFileName = L"../../_Textures/TestAlphaMap.png";
	Texture::SaveFile(saveFileName, dstTexture);
	

	//저장
	//HRESULT hr = D3DX11SaveTextureToFile(D3D::GetDC(), dstTexture, D3DX11_IFF_PNG, L"saveTest.png");
	//Check(hr);
	return saveFileName;


}

void Terrain::LoadAlphaMap()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	CreateVertexData();
	CreateIndexData();
	CreateNormalData();

	//CreateVertexBuffer
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(TerrainVertex), 0, true);
	//CreateIndexBuffer
	indexBuffer = new IndexBuffer(indices, indexCount);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(vertexBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, vertices, sizeof(TerrainVertex) * vertexCount);
	}
	D3D::GetDC()->Unmap(vertexBuffer->Buffer(), 0);
}

void Terrain::CreateVertexData()
{
	//픽셀 정보 읽기
	vector<Color> heights;
	alphaMap->ReadPixels(DXGI_FORMAT_R8G8B8A8_UNORM, &heights);

	width = alphaMap->GetWidth();
	height = alphaMap->GetHeight();


	vertexCount = width * height;

	vertices = new TerrainVertex[vertexCount];

	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			UINT index = width * z + x;
			UINT pixel = width * (height - 1 - z) + x;		//안하면 뒤집혀서 찍힘

			vertices[index].Position.x = (float)x;
			vertices[index].Position.y = heights[pixel].r * 255.0f * 0.1f;
			vertices[index].Position.z = (float)z;
			if (vertices[index].Position.y < minHeight)
				minHeight = vertices[index].Position.y;


			//uv에 spacing 곱해줘야해서 넘겨줘야 함
			//vertices[index].Uv.x = (float)x / (float)width*spacing.x;
			//vertices[index].Uv.y = (float)(height - 1 - z) / (float)height*spacing.y;
			vertices[index].Uv.x = (float)x / (float)width;
			vertices[index].Uv.y = (float)(height - 1 - z) / (float)height;
			vertices[index].Color = Color(heights[pixel].r, heights[pixel].g, heights[pixel].b, 1);
		}
	}
	int a = 0;
}

void Terrain::CreateIndexData()
{
	indexCount = (width - 1) * (height - 1) * 6;
	indices = new UINT[indexCount];

	UINT index = 0;
	for (UINT z = 0; z < height - 1; z++)
	{
		for (UINT x = 0; x < width - 1; x++)
		{
			indices[index + 0] = width * z + x;
			indices[index + 1] = width * (z + 1) + x;
			indices[index + 2] = width * z + x + 1;
			indices[index + 3] = width * z + x + 1;
			indices[index + 4] = width * (z + 1) + x;
			indices[index + 5] = width * (z + 1) + x + 1;

			index += 6;
		}
	}
}

void Terrain::CreateNormalData()
{
	/* input 동적 할당*/
	input = new InputDesc[indexCount / 3];

	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		TerrainVertex v0 = vertices[index0];
		TerrainVertex v1 = vertices[index1];
		TerrainVertex v2 = vertices[index2];

		Vector3 d1 = v1.Position - v0.Position;
		Vector3 d2 = v2.Position - v0.Position;

		Vector3 normal;
		D3DXVec3Cross(&normal, &d1, &d2);

		vertices[index0].Normal += normal;
		vertices[index1].Normal += normal;
		vertices[index2].Normal += normal;
		
		vertices[index0].Tangent += d1;
		vertices[index1].Tangent += d1;

		/* 값 넣기 */
		input[i].V0 = vertices[index0].Position;
		input[i].V1 = vertices[index1].Position;
		input[i].V2 = vertices[index2].Position;

		input[i].Index = i;
	}

	/* input 값도 받고 size도 알수 있으니 여기서 한번만 초기화 */
	size = 1 * (indexCount / 3);
	output = new OutputDesc[size];

	buffer = new StructuredBuffer
	(
		input,
		sizeof(InputDesc), size,
		sizeof(OutputDesc), size
	);


	//단위 벡터화
	for (UINT i = 0; i < vertexCount; i++)
	{
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
		D3DXVec3Normalize(&vertices[i].Tangent, &vertices[i].Tangent);
	}
}