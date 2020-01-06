#include "Framework.h"
#include "Terrain.h"


Terrain::Terrain(Shader* shader, wstring heightMap)
	:Renderer(shader), baseMap(NULL), spacing(3, 3)	//�ؽ��ĸ� 3�� Ÿ�ϸ� �ϰڴ�?
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
	CreateNormalData();		//�����ϰ� �ε����� ������ ���ϱ� ������ �̷�����

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

	/* CS�� shader*/
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
	//pickup�� �޸� ���ڷ� �ٷ� ����?
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

	//������ ���� �ʱⰪ�� false��
	bool bRaise = false;
	//�������� ���� ��ġ ����, �ʱⰪ�� �� ���� -1��
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
			//�ͷ����� �̵��� ������ �ޱ�
			GetTransform()->Position(&brushDesc.Location);
			//���콺 ��ġ ���ϱ�
			brushDesc.Location += position;
		}
	}


	//������.
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
	//�÷��̾� ��ġ�� ������ ����
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x<0 || x>=width) return -1.0f;	//x ���� ���
	if (z<0 || z>=height) return -1.0f;	//z ���� ���

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
		//Cube�� �ִ� Plane�� Forward,Right ���
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

	//Cube�� �ִ� Plane�� Up ���
	D3DXVec3Cross(&pUp, &pForward, &pRight);
	//���� ���� ȭ.
	D3DXVec3Normalize(&pUp, &pUp);

	return temp.y;
}

//���� ũ�Ⱑ �޶� ��� ����.
float Terrain::GetPickedHeight(Vector3 & position)
{
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x<0 || x>=width) return -1.0f;	//x ���� ���
	if (z<0 || z>=height) return -1.0f;	//z ���� ���

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

	//���� ������
	Vector3 start(position.x, 1000, position.z);
	//���� �� ����
	Vector3 direction(0, -1, 0);

	//�浹 ���, ������ �ʱⰪ
	Vector3 result(-1, -1, -1);

	//raypos ������ ��ġ
	//u,v ����
	//distance �浹 �������� ������ �Ÿ�
	if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;

	if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE)
		result = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;


	return result.y;
}

Vector3 Terrain::GetPickedPosition()
{
	//�������� ������
	Vector3 start, direction;
	Matrix V = Context::Get()->View();
	Matrix P = Context::Get()->Projection();

	Vector3 mouse = Mouse::Get()->GetPosition();

	Matrix world = GetTransform()->World();

	Vector3 n, f;//�ٸ� ����

	//�ٸ�
	mouse.z = 0.0f;
	Context::Get()->GetViewport()->Unproject(&n, mouse, world, V, P);

	//����
	mouse.z = 1.0f;
	Context::Get()->GetViewport()->Unproject(&f, mouse, world, V, P);

	//n���� f�� �� ����
	direction = f - n;
	start = n;


	/* CS�� �� �ֱ� */
	/* ���� ����ü �ʿ���� �ִ°� ��� */
	CS_shader->AsVector("Position")->SetFloatVector(start);
	CS_shader->AsVector("Direction")->SetFloatVector(direction);
	CS_shader->AsSRV("Input")->SetResource(buffer->SRV());
	CS_shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	/* numthreads�� 512 ũ��� ����, ���� ����� 256*256 �̶� ����� ũ�� �� */
	/* fx���Ͽ��� GroupID.x�� ������ �ּҸ� ã�µ� y,z�� ���� ��� ���ؼ� x���� ũ�� �� */
	CS_shader->Dispatch(0, 0, 512, 1, 1);

	buffer->Copy(output, sizeof(OutputDesc) * size);


	for (UINT i = 0; i < size; i++)
	{
		OutputDesc temp = output[i];
		/* ��ŷ �Ǿ����� �ش� ��ġ ����*/
		if (temp.Picked == 1)
		{
			return input[i].V0 + (input[i].V1 - input[i].V0) * temp.U + (input[i].V2 - input[i].V0) * temp.V;
		}
	}

	/*
	//x,z �𸣴� �� ����
	//�׷��� �հ��� ���� ������
	//�� �� �ﰢ�� ���� �� ��� ����
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
			//distance �浹 �������� ������ �Ÿ�
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

	//������ usage�� default����
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

	//������ usage�� default����
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
	//DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; ��� �ٲ�淡 ���� ����
	//
	Check(D3D::GetDevice()->CreateTexture2D(&dstDesc, NULL, &dstTexture));

	D3D::GetDC()->CopyResource(dstTexture, srcTexture);

//	FILE* file = fopen("Savetest.csv", "w");

	D3D11_MAPPED_SUBRESOURCE map;
	D3D::GetDC()->Map(dstTexture, 0, D3D11_MAP_READ, NULL, &map);
	{
		//memcpy�ϸ� ���� �߱淡 ���� ����
		UINT* colors = (UINT*)map.pData;

		for (UINT y = 0; y < srcDesc.Height; y++)
		{
			for (UINT x = 0; x < srcDesc.Width; x++)
			{
				//�̹��� ũ�⿡ ���� �ε��� ����
				UINT index = map.RowPitch * y / 4 + x;
				UINT vIndex = srcDesc.Width * y + x;
				//�ȼ� ����� �̱�
				UINT pixel = srcDesc.Width * (srcDesc.Height - 1 - y) + x;
				Color temp;

				temp.r = vertices[pixel].Color.r;
				temp.g = vertices[pixel].Color.g;
				temp.b = vertices[pixel].Color.b;
				//TODO : ���� �����뵵 ����..
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
	

	//����
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
	//�ȼ� ���� �б�
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
			UINT pixel = width * (height - 1 - z) + x;		//���ϸ� �������� ����

			vertices[index].Position.x = (float)x;
			vertices[index].Position.y = heights[pixel].r * 255.0f * 0.1f;
			vertices[index].Position.z = (float)z;
			if (vertices[index].Position.y < minHeight)
				minHeight = vertices[index].Position.y;


			//uv�� spacing ��������ؼ� �Ѱ���� ��
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
	/* input ���� �Ҵ�*/
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

		/* �� �ֱ� */
		input[i].V0 = vertices[index0].Position;
		input[i].V1 = vertices[index1].Position;
		input[i].V2 = vertices[index2].Position;

		input[i].Index = i;
	}

	/* input ���� �ް� size�� �˼� ������ ���⼭ �ѹ��� �ʱ�ȭ */
	size = 1 * (indexCount / 3);
	output = new OutputDesc[size];

	buffer = new StructuredBuffer
	(
		input,
		sizeof(InputDesc), size,
		sizeof(OutputDesc), size
	);


	//���� ����ȭ
	for (UINT i = 0; i < vertexCount; i++)
	{
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
		D3DXVec3Normalize(&vertices[i].Tangent, &vertices[i].Tangent);
	}
}