#include "Framework.h"
#include "TrailRenderer.h"


TrailRenderer::TrailRenderer(UINT splitCount)
	:splitCount(splitCount)
{
	Initialize();
}


TrailRenderer::~TrailRenderer()
{
	
	SafeDelete(transform)
		;
	SafeDelete(vertexBuffer);
	SafeDelete(vertices);

	SafeDelete(trailBuffer);
	SafeDelete(sTrailBuffer);

	SafeDelete(trailTexture);
	SafeDelete(sTrailSrv);
	SafeDelete(maskTexture);
	SafeDelete(sMaskSrv);
}

void TrailRenderer::Initialize()
{
	shader = SETSHADER(L"Effect/TrailEffect.fx");
	transform = new Transform(shader);
	perframe = new PerFrame(shader);

	deltaStoreTime = 0.0f;
	bViewTrail = false;
	trailDesc.TrailCount = splitCount;

	// VertexBuffer »ý¼º
	float dx = 1.0f / (float)splitCount;

	vector<VertexTexture> v;
	for (UINT i = 0; i < (UINT)splitCount; i++)
	{
		v.push_back(VertexTexture(0, 0, 0, (dx * (float)(i)), 0));
	}
	vertices = new VertexTexture[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<VertexTexture *>(vertices, vertexCount)
	);
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));

	trailBuffer = new ConstantBuffer(&trailDesc, sizeof(TrailDesc));
	sTrailBuffer = shader->AsConstantBuffer("CB_Trail");
	sTrailSrv = shader->AsSRV("TrailTexture");
	sMaskSrv = shader->AsSRV("MaskTexture");
}

void TrailRenderer::Update(Matrix parent)
{
	perframe->Update();
	
	SetAndShiftTrailMatBuffer(parent);

	for (UINT i = 0; i < mats.size(); i++)
		trailDesc.buffer[i] = mats[i];
}

void TrailRenderer::Render()
{
	transform->Render();
	perframe->Render();

	vertexBuffer->Render();

	trailBuffer->Apply();
	sTrailBuffer->SetConstantBuffer(trailBuffer->Buffer());
	if(trailTexture)
		sTrailSrv->SetResource(trailTexture->SRV());
	if(maskTexture)
		sMaskSrv->SetResource(maskTexture->SRV());

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	shader->Draw(tech, 0, splitCount);
	
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
		ImGui::Separator();
		bChange |= transform->Property();
		ImGui::Separator();
		ImGui::SliderInt("Wire", (int*)&tech, 0, 1);
		ImGui::SliderInt("Segment", (int*)&trailDesc.Segment, 2, 100);
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
	Matrix S, R, T, result;
	Vector3 scale, pos;
	Quaternion Q;
	D3DXMatrixDecompose(&scale, &Q, &pos, &insertFirstMat);
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationQuaternion(&R, &Q);
	D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
	result = R * T;
	result._14 = scale.x;
	result._24 = scale.y;
	result._34 = scale.z;
	mats.insert(mats.begin(), result);
	
	if (mats.size() > splitCount)
		mats.pop_back();	
}
