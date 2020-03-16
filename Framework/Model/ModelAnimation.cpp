#include "Framework.h"
#include "ModelAnimation.h"
#include "ModelMesh.h"

ModelAnimation::ModelAnimation(Model * model)
	: model(model)
{
	shader = model->GetShader();
	frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);
	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");

	sTransformsSRV = shader->AsSRV("TransformsMap");
	sAnimEditSRV = shader->AsSRV("AnimEditTransformMap");
	computeShader = new Shader(L"030_Collider.fx");
	
	sUav = computeShader->AsUAV("Output");
	sComputeFrameBuffer = computeShader->AsConstantBuffer("CB_AnimationFrame");

	clipTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];

}

ModelAnimation::~ModelAnimation()
{
	SafeRelease(editTexture);
	SafeRelease(sComputeFrameBuffer);
	SafeRelease(sUav);
	SafeDelete(computeBuffer);
	SafeDelete(csOutput);
	SafeDelete(computeShader);

	for (ModelClip* clip : clips)
		SafeDelete(clip);

	SafeDelete(frameBuffer);
	SafeRelease(clipTextureArray);
	SafeRelease(clipSrv);
	SafeDeleteArray(clipTransforms);
	SafeDelete(model);

}

void ModelAnimation::Update()
{
	model->Update();	
}

void ModelAnimation::Render()
{
	if (computeBuffer == NULL)
	{
		CreateComputeDesc();
	}
	if(editTexture == NULL)
	{
		CreateAnimEditTexture();
	}
	if (clipSrv != NULL)
		sTransformsSRV->SetResource(clipSrv);

	if (editSrv != NULL)
		sAnimEditSRV->SetResource(editSrv);

	model->Render();
	frameBuffer->Apply();

	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	if (computeBuffer != NULL || bChangeCS == true)
	{
		sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
		
		computeShader->AsSRV("TransformsMap")->SetResource(clipSrv);
		computeShader->AsSRV("AnimEditTransformMap")->SetResource(editSrv);
		sUav->SetUnorderedAccessView(computeBuffer->UAV());

		computeShader->Dispatch(0, 0, model->GetInstSize(), 1, 1);
		computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_MODEL_TRANSFORMS*MAX_MODEL_INSTANCE);
		bChangeCS = false;
	}
}

Matrix ModelAnimation::GetboneWorld(const UINT& instance, const UINT& boneIndex)
{

	if (csOutput == NULL)
	{
		Matrix temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}

	UINT index = instance * MAX_MODEL_TRANSFORMS + boneIndex;
	//Matrix result = csOutput[index].Result;

	return csOutput[index].Result;
}

#pragma region ������ �߰�

void ModelAnimation::ReadClip(const wstring& file, const wstring& directoryPath)
{
	if (clips.size() >= MAX_ANIMATION_CLIPS)
		return;
	wstring noextfile = Path::GetFilePathWithoutExtension(file);
	wstring loadPath = directoryPath + noextfile + L".clip";

	BinaryReader* r = new BinaryReader();
	r->Open(loadPath);


	ModelClip* clip = new ModelClip();

	clip->name = String::ToWString(r->String());
	clip->duration = r->Float();
	clip->frameRate = r->Float();
	clip->frameCount = r->UInt();

	UINT keyframesCount = r->UInt();
	for (UINT i = 0; i < keyframesCount; i++)
	{
		ModelKeyframe* keyframe = new ModelKeyframe();
		keyframe->BoneName = String::ToWString(r->String());
		
		UINT size = r->UInt();
		if (size > 0)
		{
			keyframe->Transforms.assign(size, ModelKeyframeData());

			void* ptr = (void *)&keyframe->Transforms[0];
			r->Byte(&ptr, sizeof(ModelKeyframeData) * size);
		}

		clip->keyframeMap[keyframe->BoneName] = keyframe;
	}

	r->Close();
	SafeDelete(r);

	clips.push_back(clip);

	UpdateTextureArray();
}

void ModelAnimation::SaveChangedClip(const UINT& clip, const wstring& file, const wstring& directoryPath, bool bOverwrite)
{
	//����� �����÷θ�
	if (clip >= clips.size())
		return;

	wstring noextfile = Path::GetFilePathWithoutExtension(file);
	wstring savePath = directoryPath + noextfile + L".clip";
	if (bOverwrite == false)
	{
		if (Path::ExistFile(savePath) == true)
			return;
	}
	
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	ModelClip* saveclip=clips[clip];

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->String(String::ToString( saveclip->name));
	w->Float(saveclip->duration);
	w->Float(saveclip->frameRate);
	w->UInt(saveclip->frameCount);
	
	UINT keyframesCount = saveclip->keyframeMap.size();
	w->UInt(keyframesCount);

	for (auto it = saveclip->keyframeMap.begin(); it != saveclip->keyframeMap.end(); it++)
	{
		wstring boneName = it->first;
		w->String(String::ToString(boneName));

		ModelKeyframe* keyframe = it->second;
		UINT size = keyframe->Transforms.size();
		w->UInt(size);
		vector<ModelKeyframeData> datas;
		ModelBone* bone = model->BoneByName(boneName);
		
		//TODO: ���� ����
		Matrix edit = bone->GetEditTransform()->Local();
		for(UINT f = 0; f < size; f++)
		{
			ModelKeyframeData& data = keyframe->Transforms[f];
			ModelKeyframeData editedData;

			Matrix S, R, T, W;
			D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
			D3DXMatrixRotationQuaternion(&R, &data.Rotation);
			D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);
				
			if (bone != NULL)
			{
				W = S * R * T * edit;
			}
			else
			{
				W = S * R * T;
			}
			D3DXMatrixDecompose(&editedData.Scale, &editedData.Rotation, &editedData.Translation, &W);
			editedData.Time	= data.Time;
			datas.emplace_back(editedData);
		}
		if (size > 0)
		{
			w->Byte(&datas[0], sizeof(ModelKeyframeData) * datas.size());
		}
	}

	w->Close();
	SafeDelete(w);
}

void ModelAnimation::AddClip(const wstring& file, const wstring& directoryPath)
{
	ReadClip(file, directoryPath);

	int addClipIndex = ClipCount() - 1;
	CreateClipTransform(addClipIndex);

	ModelClip* clip = ClipByIndex(addClipIndex);
	
	///* ��ȯ ���� �ؽ��� �������� �ڽ� ���� */
	//D3D11_BOX destRegion;
	///* �� �ϳ��� ũ�� */
	//destRegion.left = 0;
	//destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	//destRegion.front = addClipIndex;
	//destRegion.back = addClipIndex + 1;

	//for(UINT y = 0; y < clip->FrameCount(); y++)
	//{
	//	/* ������ frame�� ��ġ  */
	//	destRegion.top = y;
	//	destRegion.bottom = y + 1;
	//	/* ������Ʈ */
	//	D3D::GetDC()->UpdateSubresource
	//	(
	//		clipTexture,
	//		0,
	//		&destRegion,
	//		clipTransforms[addClipIndex].Transform[y],
	//		sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
	//		0
	//	);
	//}
}

void ModelAnimation::AddSocket(const int& parentBoneIndex, const wstring& bonename)
{
	bChangeCS = true;
	UINT index = model->BoneCount();
	
	model->AddSocket(parentBoneIndex, bonename);
	
	UINT clipcount = ClipCount();

	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	//���� CS���� ����
	//destRegion.left = 4 * (index);
	//destRegion.right = 4 * (index+1);
	for(UINT x = 0; x < clipcount; x++)
	{

		ModelClip* clip = ClipByIndex(x);

		destRegion.front = x;
		destRegion.back = x + 1;

		for(UINT y = 0; y < clip->FrameCount(); y++)
		{
			memcpy(clipTransforms[x].Transform[y][index], clipTransforms[x].Transform[y][parentBoneIndex], sizeof(Matrix)); //����
			destRegion.top = y;
			destRegion.bottom = y + 1;
			/* ������Ʈ */
			D3D::GetDC()->UpdateSubresource
			(
				clipTextureArray,
				//clipTexture,
				0,
				&destRegion,
				clipTransforms[x].Transform[y],
				sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
				0
			);
		}
	}
	ModelBone* bone = model->BoneByIndex(index);
	for(UINT x = 0; x < clipcount; x++)
	{
		Matrix global = bone->GetEditTransform()->World();
		// ���� ��� ������ �ʿ��� ���·� �������ؼ� �ѱ�
		Matrix S, R, T, result;
		Vector3 scale, pos;
		Quaternion Q;
		D3DXMatrixDecompose(&scale, &Q, &pos, &global);
		D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
		D3DXMatrixRotationQuaternion(&R, &Q);
		D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
		result = R * T;
		result._14 = scale.x;
		result._24 = scale.y;
		result._34 = scale.z;
		animEditTrans[x][index] = result;
		for (ModelBone* child : bone->Childs())
		{
			UpdateChildBones(index, child->Index(), x);
		}

		D3D11_BOX destRegion;
		/* �� �ϳ��� ũ�� */
		destRegion.left = 0;
		destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
		/* ���� �ν��Ͻ� ���� ��ġ */
		destRegion.top = x;
		destRegion.bottom = x + 1;
		destRegion.front = 0;
		destRegion.back = 1;

		/* ������Ʈ */
		D3D::GetDC()->UpdateSubresource
		(
			editTexture,
			0,
			&destRegion,
			animEditTrans[x],
			sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
			0
		);
	}
}

#pragma endregion

#pragma region AnimationData

void ModelAnimation::PlayAnim(const UINT& instance)
{
	{
		TweenDesc& desc = tweenDesc[instance];
		ModelClip* clip = ClipByIndex(desc.Curr.Clip);

		//���� �ִϸ��̼�
		{
			desc.Curr.RunningTime += Time::Delta();
			float time = 1.0f / clip->FrameRate() / desc.Curr.Speed;
			if (desc.Curr.Time >= 1.0f)
			{
				desc.Curr.RunningTime = 0.0f;

				desc.Curr.CurrFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();
				desc.Curr.NextFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();
			}
			desc.Curr.Time = desc.Curr.RunningTime / time;
		}

		if (desc.Next.Clip > -1)
		{
			ModelClip* nextClip = ClipByIndex(desc.Next.Clip);

			desc.RunningTime += Time::Delta();
			desc.TweenTime = desc.RunningTime / desc.TakeTime;

			if (desc.TweenTime >= 1.0f)
			{
				desc.Curr = desc.Next;

				desc.Next.Clip = -1;
				desc.Next.CurrFrame = 0;
				desc.Next.NextFrame = 0;
				desc.Next.Time = 0;
				desc.Next.RunningTime = 0.0f;

				desc.RunningTime = 0.0f;
				desc.TweenTime = 0.0f;
			}
			else
			{
				desc.Next.RunningTime += Time::Delta();
				float time = 1.0f / nextClip->FrameRate() / desc.Next.Speed;
				if (desc.Next.Time >= 1.0f)
				{
					desc.Next.RunningTime = 0.0f;

					desc.Next.CurrFrame = (desc.Next.CurrFrame + 1) % nextClip->FrameCount();
					desc.Next.NextFrame = (desc.Next.CurrFrame + 1) % nextClip->FrameCount();
				}
				desc.Next.Time = desc.Next.RunningTime / time;
			}
		}
	}//for(i)

	frameBuffer->Apply();
	

}

void ModelAnimation::PlayClip(const UINT& instance, const UINT& clip, float speed, float takeTime)
{
	tweenDesc[instance].TakeTime = takeTime;
	tweenDesc[instance].Next.Clip = clip;
	tweenDesc[instance].Next.Speed = speed;
}

UINT ModelAnimation::GetCurrClip(const UINT& instance)
{
	return tweenDesc[instance].Next.Clip>-1? tweenDesc[instance].Next.Clip: tweenDesc[instance].Curr.Clip;
}

void ModelAnimation::SetFrame(const UINT& instance, int frameNum)
{
	ModelClip* currClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	tweenDesc[instance].Curr.CurrFrame = frameNum % currClip->FrameCount();
	tweenDesc[instance].Curr.NextFrame = (frameNum + 1) % currClip->FrameCount();
}

UINT ModelAnimation::GetFrameCount(const UINT& instance)
{
	ModelClip* resultClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	if(tweenDesc[instance].Next.Clip>-1)
		resultClip = ClipByIndex(tweenDesc[instance].Next.Clip);
	return resultClip->FrameCount();
}

#pragma endregion

ModelClip * ModelAnimation::ClipByName(const wstring& name)
{
	for (ModelClip* clip : clips)
	{
		if (clip->name == name)
			return clip;
	}

	return NULL;
}

#pragma region CreateDataRegion
/* 
	1. �� �Լ��� ����� �ؽ��Ĵ� ������ �ִϸ��̼��� Ʈ���� ���� ���� �Ұ���
	2. Ŭ���� �߰� �ɶ����� ����� �Ұ���
	... �������� �� ���ƿԴ�...
*/
void ModelAnimation::UpdateTextureArray()
{
	// ���� �߰��� �ָ� Ŭ�������� ����� �ؽ��� �����.
	UINT c = clips.size() - 1;
	CreateClipTransform(c);

	SafeRelease(clipTextureArray);
	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = clips.size();
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		
		void* p = VirtualAlloc(NULL, pageSize*clips.size(), MEM_RESERVE, PAGE_READWRITE); //����

		for(UINT c = 0; c < clips.size(); c++)
		{
			for(UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
			{
				UINT start = c * pageSize;
				void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * y * sizeof(Matrix) + start;

				VirtualAlloc(temp, MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE); //���� �Ҵ�
				memcpy(temp, clipTransforms[c].Transform[y], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //����
			}
		}

		D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA[clips.size()];
		for(UINT c = 0; c < clips.size(); c++)
		{
			void* temp = (BYTE *)p + c * pageSize;

			subResource[c].pSysMem = temp;
			subResource[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
			subResource[c].SysMemSlicePitch = pageSize;
		}
		Check(D3D::GetDevice()->CreateTexture2D(&desc, subResource, &clipTextureArray));

		SafeDeleteArray(subResource);
		//free(p);
		VirtualFree(p, 0, MEM_RELEASE);
	}

	//CreateSRV
	{
		D3D11_TEXTURE2D_DESC desc;
		clipTextureArray->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = clips.size();

		Check(D3D::GetDevice()->CreateShaderResourceView(clipTextureArray, &srvDesc, &clipSrv));
	}
}

void ModelAnimation::CreateClipTransform(const UINT& index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS];
	
	ModelClip* clip = ClipByIndex(index);
	for(UINT f = 0; f < clip->FrameCount(); f++)
	{
		for(UINT b = 0; b < model->BoneCount(); b++)
		{
			ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;
			///* 
			//	1. �ִϸ��̼ǿ��� ����ķ� ���� 
			//	2. Ű������ �����ʹ� ������. �׷��� �θ� ���� �����ַ��°�
			//*/

			int parentIndex = bone->ParentIndex();
			if (parentIndex < 0)
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];

			Matrix animation;
			ModelKeyframe* frame = clip->Keyframe(bone->Name());
			if (frame != NULL)
			{
				ModelKeyframeData& data = frame->Transforms[f];

				Matrix S, R, T;
				D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
				D3DXMatrixRotationQuaternion(&R, &data.Rotation);
				D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

				animation = S * R * T;

				bones[b] = animation * parent;
				//clipTransforms[index].Transform[f][b] = invGlobal*bones[b];
			}
			else
			{
				bones[b] = parent;
				//clipTransforms[index].Transform[f][b] = bone->BoneWorld()*bones[b];
			}
			Matrix S,R,T,result;
			Vector3 scale, pos;
			Quaternion Q;
			D3DXMatrixDecompose(&scale, &Q, &pos, &bones[b]);
			D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
			D3DXMatrixRotationQuaternion(&R, &Q);
			D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
			result = R * T;
			result._14 = scale.x;
			result._24 = scale.y;
			result._34 = scale.z;

			clipTransforms[index].Transform[f][b] = result;

			//clipTransforms[index].Transform[f][b] = bones[b];
		}//for(b)
	}//for(f)
}

void ModelAnimation::CreateNoClipTransform(const UINT& index)
{
	for(UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
		memcpy(clipTransforms[index].Transform[y], clipTransforms[0].Transform[0], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //����
}

void ModelAnimation::CreateComputeDesc()
{
	UINT outSize = MAX_MODEL_TRANSFORMS*MAX_MODEL_INSTANCE;
	
	computeBuffer = new StructuredBuffer
	(
		NULL,
		sizeof(CS_OutputDesc), outSize,
		true
	);

	if (csOutput == NULL)
	{
		csOutput = new CS_OutputDesc[outSize];

		for(UINT i = 0; i < outSize; i++)
			D3DXMatrixIdentity(&csOutput[i].Result);
	}
}
#pragma endregion


#pragma region �ִϸ��̼� Ŭ�� ��ȭ �ؽ��� ���� �� ��ȭ ���� ����

void ModelAnimation::CreateAnimEditTexture()
{
	// �ش� ���۰� �ִϸ��̼��� ��ȭ�� ����Ѵ�.
	// �⺻ ���� identity
	for(UINT b = 0; b < model->BoneCount(); b++)
		for(UINT c = 0; c < MAX_ANIMATION_CLIPS; c++)
		{
			D3DXMatrixIdentity(&animEditTrans[c][b]);
			animEditTrans[c][b]._14 = animEditTrans[c][b]._24 = animEditTrans[c][b]._34 = 1;
		}

	//CreateTexture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;
		desc.Height = MAX_ANIMATION_CLIPS;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;

		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = animEditTrans;
		subResource.SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
		subResource.SysMemSlicePitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix)*MAX_ANIMATION_CLIPS;

		Check(D3D::GetDevice()->CreateTexture2D(&desc, &subResource, &editTexture));
	}

	//Create SRV
	{
		D3D11_TEXTURE2D_DESC desc;
		editTexture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = desc.Format;

		Check(D3D::GetDevice()->CreateShaderResourceView(editTexture, &srvDesc, &editSrv));
	}

	//for (ModelMesh* mesh : model->Meshes())
	//	mesh->AnimEditSrv(editSrv);
}

void ModelAnimation::UpdateBoneTransform(const UINT& part, const UINT& clipID, Transform* transform)
{
	if (editTexture == NULL || model == NULL)
		return;
	//Ŭ�� ��ȭ�� ���� ������ ��ġ ��ȭ ������ ���� CS�� �ٽ� ����
	bChangeCS = true;

	ModelBone* bone = model->BoneByIndex(part);

	Matrix trans = transform->World();
	bone->GetEditTransform()->Local(trans);
	Matrix global = bone->GetEditTransform()->World();
	// ���� ��� ������ �ʿ��� ���·� �������ؼ� �ѱ�
	Matrix S, R, T, result;
	Vector3 scale, pos;
	Quaternion Q;
	D3DXMatrixDecompose(&scale, &Q, &pos, &global);
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationQuaternion(&R, &Q);
	D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
	result = R * T;
	result._14 = scale.x;
	result._24 = scale.y;
	result._34 = scale.z;
	animEditTrans[clipID][part] = result;
	for (ModelBone* child : bone->Childs())
	{
		UpdateChildBones(part, child->Index(), clipID);
	}

	D3D11_BOX destRegion;
	/* �� �ϳ��� ũ�� */
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	/* ���� �ν��Ͻ� ���� ��ġ */
	destRegion.top = clipID;
	destRegion.bottom = clipID + 1;
	destRegion.front = 0;
	destRegion.back = 1;

	/* ������Ʈ */
	D3D::GetDC()->UpdateSubresource
	(
		editTexture,
		0,
		&destRegion,
		animEditTrans[clipID],
		sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
		0
	);
}

void ModelAnimation::UpdateChildBones(const UINT& parentID, const UINT& childID, const UINT& clipID)
{
	ModelBone* bone = model->BoneByIndex(childID);

	bone->GetEditTransform()->Update();
	Matrix global = bone->GetEditTransform()->World();
	Matrix S, R, T, result;
	Vector3 scale, pos;
	Quaternion Q;
	D3DXMatrixDecompose(&scale, &Q, &pos, &global);
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationQuaternion(&R, &Q);
	D3DXMatrixTranslation(&T, pos.x, pos.y, pos.z);
	result = R * T;
	result._14 = scale.x;
	result._24 = scale.y;
	result._34 = scale.z;
	animEditTrans[clipID][childID] = result;
	for (ModelBone* child : bone->Childs())
	{
		UpdateChildBones(childID, child->Index(), clipID);
	}
}

#pragma endregion