#include "Framework.h"
#include "FFTOcean.h"


#define MESH_INDEX_2D(x, y)	(((y) + vert_rect.bottom) * (MeshDim + 1) + (x) + vert_rect.left)

FFTOcean::FFTOcean(InitializeInfo & info)
	:Renderer(info.shader)
{
	this->info = info;
	Topology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	sHeightTexture = shader->AsSRV("HeightMap");
	/* 높이맵 불러오기 */
	heightMap = new Texture(info.heightMap);
	sHeightTexture->SetResource(heightMap->SRV());
	shader = info.shader;
	CreateOceanSimAndRender();
	InitializeOcean();
}

FFTOcean::~FFTOcean()
{
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(PerCallCB);
	SafeDelete(frustum);
	SafeDelete(heightMap);

	SafeRelease(FresnelMap);
	SafeRelease(SRV_Fresnel);
	SafeRelease(SRV_Perlin);
	SafeRelease(SRV_ReflectCube);
	SafeRelease(SRV_Displacemnet);
	SafeRelease(SRV_Gradient);

}

void FFTOcean::InitializeOcean()
{


	PerCallCB = new ConstantBuffer(&callDesc, sizeof(BufferDesc));
	sPerCallCB = shader->AsConstantBuffer("CB_Ocean");



	width = this->heightMap->GetWidth() - 1;
	height = this->heightMap->GetHeight() - 1;


	patchVertexRows = (width / info.CellsPerPatch) + 1;
	patchVertexCols = (height / info.CellsPerPatch) + 1;

	vertexCount = patchVertexRows * patchVertexCols;
	faceCount = (patchVertexRows - 1) * (patchVertexCols - 1);
	indexCount = faceCount * 4;

	CreateVertexData();
	CreateIndexData();
/*
	QuadTreeNode* root = CreateQuadTreeData(NULL, Vector2(0, 0), Vector2(width, height));
	quadTree = new QuadTree(root);
*/
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexOcean));
	indexBuffer = new IndexBuffer(indices, indexCount);

	callDesc.HalfOceanSize = Vector2((float)(width+1), (float)(height+1))*0.5f;


}

bool FFTOcean::InBounds(UINT row, UINT col)
{
	return row >= 0 && row < height && col >= 0 && col < width;

}


void FFTOcean::CreateVertexData()
{
	vertices = new VertexOcean[vertexCount];

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
}

void FFTOcean::CreateIndexData()
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

void FFTOcean::CreateOceanSimAndRender()
{

	OceanParameter ocean_param;

	// The size of displacement map. In this sample, it's fixed to 512.
	ocean_param.dmap_dim = 512;
	// The side length (world space) of square patch
	ocean_param.patch_length = 512.0f*2.0f;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale = 0.8f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude = 1.35f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir = Vector2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed = 100.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	ocean_param.choppy_scale = 1.3f;

	oceanSimulator = new FFTOceanSimulator(ocean_param);
	InitRenderResource(ocean_param);
}

void FFTOcean::InitRenderResource(const OceanParameter & ocean_param)
{
	for (UINT i = 0; i < 4; i++)
	{
		renderSrvs[i] = new Render2D();
		renderSrvs[i]->GetTransform()->Position(D3D::Width() - 100 * i-60, 60, 0);
		renderSrvs[i]->GetTransform()->Scale(100, 100, 1);
	}

	PatchLength = ocean_param.patch_length;
	DisplaceMapDim = ocean_param.dmap_dim;
	WindDir = ocean_param.wind_dir;
	
	CreateFresnelMap();
	LoadTextures();

	//////////////////////////////////////////////////////////////////////////////
	// Grid side length * 2
	shadingDesc.TexelLength_x2 = PatchLength / DisplaceMapDim * 2;;
	// Color
	shadingDesc.SkyColor = SkyColor;
	shadingDesc.WaterbodyColor = WaterbodyColor;
	// Sun
	shadingDesc.SunColor = SunColor;
	shadingDesc.SunDir = Context::Get()->LightDirection();
	shadingDesc.SunDir.x = -shadingDesc.SunDir.x;
	// Texcoord
	shadingDesc.UVScale = 1.0f / PatchLength;
	shadingDesc.UVOffset = 0.5f / DisplaceMapDim;
	// Perlin
	shadingDesc.PerlinSize = PerlinSize;
	shadingDesc.PerlinAmplitude = PerlinAmplitude;
	shadingDesc.PerlinGradient = PerlinGradient;
	shadingDesc.PerlinOctave = PerlinOctave;
	// Multiple reflection workaround
	shadingDesc.BendParam = BendParam;
	// Sun streaks
	shadingDesc.Shineness = Shineness;
	ShadingCB = new ConstantBuffer(&shadingDesc, sizeof(Const_Shading));
	//////////////////////////////////////////////////////////////////////////////
}

void FFTOcean::Update(float time)
{
	Super::Update();

	transform->Update();
	oceanSimulator->UpdateDisplacementMap(time);
	SRV_Displacemnet= oceanSimulator->GetDisplacementMap();
	SRV_Gradient = oceanSimulator->GetGradientMap();
}

void FFTOcean::Render(float time)
{
	Super::Render();

	// Textures
	shader->AsSRV("TexDisplacement")->SetResource(SRV_Displacemnet);
	shader->AsSRV("TexPerlin")->SetResource(SRV_Perlin);
	shader->AsSRV("TexGradient")->SetResource(SRV_Gradient);
	shader->AsSRV("TexFresnel")->SetResource(SRV_Fresnel);
	shader->AsSRV("TexReflectCube")->SetResource(SRV_ReflectCube);

	shadingDesc.SunDir = Context::Get()->LightDirection();
	shadingDesc.SunDir.x = -shadingDesc.SunDir.x;
	ShadingCB->Apply();
	shader->AsConstantBuffer("CB_Shading")->SetConstantBuffer(ShadingCB->Buffer());
	
	PerCallCB->Apply();
	sPerCallCB->SetConstantBuffer(PerCallCB->Buffer());

	// Constant PerlinSpeed need to be adjusted mannually
	Vector2 perlin_move = -WindDir * time * PerlinSpeed;
	callDesc.PerlinMovement = perlin_move;


	// Update constant buffer
	//PerCallCB;
	PerCallCB->Apply();
	shader->AsConstantBuffer("CB_PerCall")->SetConstantBuffer(PerCallCB->Buffer());
	shader->DrawIndexed(0, Pass(), indexCount);
}


void FFTOcean::CreateFresnelMap()
{
	DWORD* buffer = new DWORD[FRESNEL_TEX_SIZE];
	for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
	{
		float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
		// Using water's refraction index 1.33
		DWORD fresnel = (DWORD)(D3DXFresnelTerm(cos_a, 1.33f) * 255);

		DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), SkyBlending) * 255);

		buffer[i] = (sky_blend << 8) | fresnel;
	}

	D3D11_TEXTURE1D_DESC tex_desc;
	tex_desc.Width = FRESNEL_TEX_SIZE;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = buffer;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	D3D::GetDevice()->CreateTexture1D(&tex_desc, &init_data, &FresnelMap);
	assert(FresnelMap);

	SafeDeleteArray(buffer);

	// Create shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srv_desc.Texture1D.MipLevels = 1;
	srv_desc.Texture1D.MostDetailedMip = 0;

	D3D::GetDevice()->CreateShaderResourceView(FresnelMap, &srv_desc, &SRV_Fresnel);
	assert(SRV_Fresnel);
}


void FFTOcean::LoadTextures()
{
	Perlin = new Texture(L"Ocean/perlin_noise.dds");
	SRV_Perlin = Perlin->SRV();
	renderSrvs[1]->SRV(SRV_Perlin);

	ReflectionCube = new Texture(L"Ocean/reflect_cube.dds");
	//ReflectionCube = new TextureCube((Vector3&)Vector3(0, 0, 0), 512, 512);
	SRV_ReflectCube = ReflectionCube->SRV();
}

void FFTOcean::RenderFFT()
{
	//oceanSimulator->RenderFFT();
	renderSrvs[0]->SRV(SRV_Displacemnet);
	renderSrvs[0]->Render();

	renderSrvs[1]->Render();

	renderSrvs[2]->SRV(SRV_Gradient);
	renderSrvs[2]->Render();

	renderSrvs[3]->SRV(SRV_Fresnel);
	renderSrvs[3]->Render();

}

void FFTOcean::Property()
{
	ImGui::Begin("FFT_OceanProperty");
	{
		static bool bWireFrame = false;
		ImGui::Checkbox("ToggleWireFrame", &bWireFrame);
		Pass(bWireFrame ? 1 : 0);
		ImGui::Separator();
		ImGui::ColorEdit4("FFTSky", (float*)shadingDesc.SkyColor);
		ImGui::ColorEdit4("FFTWater", (float*)shadingDesc.WaterbodyColor);
		ImGui::Separator();
		ImGui::SliderFloat("PerlinSpeed", &PerlinSpeed,0.01f,10.0f);
		ImGui::SliderFloat("PerlinSize", &shadingDesc.PerlinSize,1.0f,100.0f);
		ImGui::SliderFloat3("PerlinAmplitude", (float*)&shadingDesc.PerlinAmplitude,10.0f,1000.0f);
		ImGui::SliderFloat3("PerlinGradient", (float*)&shadingDesc.PerlinGradient,0.01f,100.0f);
		ImGui::SliderFloat3("PerlinOctave", (float*)&shadingDesc.PerlinOctave,0.01f,100.0f);
		ImGui::Separator();
		ImGui::SliderFloat2("WindDir", (float*)&WindDir, -1.0f, 1.0f);
		//D3DXVec2Normalize(&WindDir, &WindDir);
		ImGui::SliderFloat3("BendParam", (float*)&BendParam, -1.0f, 1.0f);
		//D3DXVec3Normalize(&BendParam, &BendParam);

		ImGui::Separator();
		Vector3 pos, scale;
		GetTransform()->Position(&pos);
		ImGui::SliderFloat3("OceanPos", (float*)&pos, -512, 512);
		GetTransform()->Position(pos);
		GetTransform()->Scale(&scale);
		ImGui::SliderFloat3("OceanScale", (float*)&scale, 0.1f, 5.0f);
		GetTransform()->Scale(scale);
	}
	ImGui::End();
}

//
//QuadTreeNode * FFTOcean::CreateQuadTreeData(QuadTreeNode * parent, Vector2 & TopLeft, Vector2 & BottomRight)
//{
//	const float tolerance = 0.01f;
//	Vector2 minMaxY(-1,1);
//
//	float minX = TopLeft.x * info.CellSpacing - width * 0.5f;
//	float maxX = BottomRight.x * info.CellSpacing - width * 0.5f;
//	float minZ = -TopLeft.y * info.CellSpacing + height * 0.5f;
//	float maxZ = -BottomRight.y * info.CellSpacing + height * 0.5f;
//
//	// adjust the bounds to get a very slight overlap of the bounding boxes
//	minX -= tolerance;
//	maxX += tolerance;
//	minZ += tolerance;
//	maxZ -= tolerance;
//	QuadTreeNode* quadNode = new QuadTreeNode();
//	if (parent != NULL)
//		quadNode->SetParent(parent);
//
//	Vector3 minV3(minX, minMaxY.x, maxZ);
//	Vector3 maxV3(maxX, minMaxY.y, minZ);
//	Vector3 scale = maxV3 - minV3;
//	Vector3 position = (maxV3 + minV3)*0.5f;
//	Transform* transform = new Transform();
//	transform->Position(position);
//	transform->Scale(scale);
//	Collider* collider = new Collider(transform);
//
//	quadNode->SetCollider(collider);
//	float narrow = (BottomRight.x - TopLeft.x)*0.5f;
//	float depth = (BottomRight.y - TopLeft.y) *0.5f;
//
//	float TileSize = shadingDesc.TexelLength_x2;
//	if (narrow >= TileSize && depth >= TileSize) {
//		quadNode->AddChild(CreateQuadTreeData(quadNode, TopLeft, Vector2(TopLeft.x + narrow, TopLeft.y + depth)));
//		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x + narrow, TopLeft.y), Vector2(BottomRight.x, TopLeft.y + depth)));
//		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x, TopLeft.y + depth), Vector2(TopLeft.x + depth, BottomRight.y)));
//		quadNode->AddChild(CreateQuadTreeData(quadNode, Vector2(TopLeft.x + narrow, TopLeft.y + depth), BottomRight));
//	}
//
//	return quadNode;
//}
