#include "Framework.h"
#include "TrailRenderer.h"



TrailRenderer::TrailRenderer(UINT splitCount)
	:splitCount(splitCount)
{
	Initialize();
}


TrailRenderer::~TrailRenderer()
{
	SafeDelete(shader);
	SafeDelete(transform)
		;
	SafeDelete(vertexBuffer);
	SafeDelete(vertices);
	SafeDelete(indices);

	SafeDelete(trailBuffer);
	SafeDelete(sTrailBuffer);

	SafeDelete(trailTexture);
	SafeDelete(sTrailSrv);
	SafeDelete(maskTexture);
	SafeDelete(sMaskSrv);
}

void TrailRenderer::Initialize()
{
	shader = new Shader(L"Effect/TrailEffect.fx");
	transform = new Transform(shader);
	perframe = new PerFrame(shader);

	deltaStoreTime = 0.0f;
	bViewTrail = false;
	trailDesc.TrailCount = splitCount;

	// VertexBuffer 持失
	float w = 0.5f;
	float h = 0.5f;
	float dx = 1.0f / (float)splitCount;

	vector<VertexTexture> v;
	for (UINT i = 0; i < (UINT)splitCount; i++)
	{
		v.push_back(VertexTexture(-w, -h, 0, (dx * (float)(i + 0)), 1));
		v.push_back(VertexTexture(-w, +h, 0, (dx * (float)(i + 0)), 0));
		v.push_back(VertexTexture(+w, -h, 0, (dx * (float)(i + 1)), 1));
		v.push_back(VertexTexture(+w, +h, 0, (dx * (float)(i + 1)), 0));
	}
	vertices = new VertexTexture[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<VertexTexture *>(vertices, vertexCount)
	);
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));

	// IndexBuffer 持失

	indexCount = 6* splitCount;
	indices = new UINT[indexCount];
	for (UINT i = 0; i < (UINT)splitCount; i++)
	{
		indices[i * 6 + 0] = i*4+0;
		indices[i * 6 + 1] = i*4+1;
		indices[i * 6 + 2] = i*4+2;
		indices[i * 6 + 3] = i*4+2;
		indices[i * 6 + 4] = i*4+1;
		indices[i * 6 + 5] = i*4+3;

	}
	indexBuffer = new IndexBuffer(indices, indexCount);


	trailBuffer = new ConstantBuffer(&trailDesc, sizeof(TrailDesc));
	sTrailBuffer = shader->AsConstantBuffer("CB_Trail");
	sTrailSrv = shader->AsSRV("TrailTexture");
	sMaskSrv = shader->AsSRV("MaskTexture");
}

void TrailRenderer::Update(Matrix parent)
{
	//transform->Update();
	perframe->Update();
	deltaStoreTime += Time::Delta();

	if (deltaStoreTime >= 0.0005f)
	{
		deltaStoreTime -= 0.0005f;

		//D3DXMatrixTranspose(&parent, &parent);
		SetAndShiftTrailMatBuffer(parent);
	}

	for (UINT i = 0; i < mats.size(); i++)
		trailDesc.buffer[i] = mats[i];
}

void TrailRenderer::Render()
{
	transform->Render();
	perframe->Render();

	vertexBuffer->Render();
	indexBuffer->Render();

	trailBuffer->Apply();
	sTrailBuffer->SetConstantBuffer(trailBuffer->Buffer());
	if(trailTexture)
		sTrailSrv->SetResource(trailTexture->SRV());
	if(maskTexture)
		sMaskSrv->SetResource(maskTexture->SRV());

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, 0, 6 * splitCount);
}

void TrailRenderer::ClearTrail(Matrix mat)
{
	for (UINT i = 0; i < splitCount; i++)
		trailDesc.buffer[i] = mat;
}

void TrailRenderer::TrailTexture(string path, string dir)
{
	TrailTexture(String::ToWString(path), String::ToWString(dir));
}

void TrailRenderer::TrailTexture(wstring path, wstring dir)
{
	SafeDelete(trailTexture);

	trailTexture = new Texture(path, dir);
}

void TrailRenderer::MaskTexture(string path, string dir)
{
	MaskTexture(String::ToWString(path), String::ToWString(dir));
}

void TrailRenderer::MaskTexture(wstring path, wstring dir)
{
	SafeDelete(maskTexture);

	maskTexture = new Texture(path, dir);
}

bool TrailRenderer::Property()
{
	bool bChange;
	{
		ImGui::Text("TrailRender Property");
		bChange |= transform->Property();
		ImGui::Separator();

		ID3D11ShaderResourceView* srv = NULL;
		if (trailTexture)
			srv = trailTexture->SRV();
		
		ImGui::PushID(trailTexture);
		bChange |= ImGui::ImageButton(srv, ImVec2(50, 50));
		Item* item;
		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				TrailTexture(file, dir);
			}
		}
		ImGui::PopID();

		ImGui::SameLine();
		srv = NULL;
		if (maskTexture)
			srv = maskTexture->SRV();
		ImGui::PushID(maskTexture);
		bChange |= ImGui::ImageButton(srv, ImVec2(50, 50));
		{
			item = DragDrop::GetDragDropPayload_Item(DragDropPayloadType::Texture);
			if (item != NULL)
			{
				string dir = Path::GetDirectoryName(item->filePath);
				string file = Path::GetFileName(item->filePath);
				MaskTexture(file, dir);
			}
		}
		ImGui::PopID();

	}
	return bChange;
}

void TrailRenderer::SetAndShiftTrailMatBuffer(Matrix & insertFirstMat)
{
	mats.insert(mats.begin(),insertFirstMat);
	if (mats.size() > splitCount)
		mats.pop_back();
	//memcpy
	//(
	//	&trailDesc.buffer[1],
	//	&trailDesc.buffer[0],
	//	sizeof(D3DXMATRIX) * (splitCount - 1)
	//);
	//trailDesc.buffer[0] = insertFirstMat;
}
