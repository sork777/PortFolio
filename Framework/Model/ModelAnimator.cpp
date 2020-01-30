#include "Framework.h"
#include "ModelAnimator.h"
#include "ModelMesh.h"

ModelAnimator::ModelAnimator(Model * model)
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
}

ModelAnimator::~ModelAnimator()
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
	SafeRelease(clipTexture);
	SafeRelease(clipSrv);
	SafeDeleteArray(clipTransforms);
	SafeDelete(model);

}

void ModelAnimator::Update()
{
	model->Update();	
}

void ModelAnimator::Render()
{
	if (clipTexture == NULL)
	{
		CreateComputeDesc();
		CreateTexture();

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

Matrix ModelAnimator::GetboneWorld(UINT instance, UINT boneIndex)
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

#pragma region 데이터 추가

void ModelAnimator::ReadClip(wstring file, wstring directoryPath)
{
	if (clips.size() >= MAX_ANIMATION_CLIPS)
		return;
	file = Path::GetFilePathWithoutExtension(file);
	file = directoryPath + file + L".clip";

	BinaryReader* r = new BinaryReader();
	r->Open(file);


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
}

void ModelAnimator::SaveChangedClip(UINT clip, wstring file, wstring directoryPath, bool bOverwrite)
{
	//사이즈가 오버플로면
	if (clip >= clips.size())
		return;

	file = Path::GetFilePathWithoutExtension(file);
	wstring savePath = directoryPath + file + L".clip";
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
		
		//TODO: 나중 변경
		Matrix edit = bone->GetEditTransform()->Local();
		for (UINT f = 0; f < size; f++)
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

void ModelAnimator::AddClip(wstring file, wstring directoryPath)
{
	ReadClip(file, directoryPath);

	int addClipIndex = ClipCount() - 1;
	CreateClipTransform(addClipIndex);

	ModelClip* clip = ClipByIndex(addClipIndex);
	
	/* 변환 해줄 텍스쳐 데이터의 박스 영역 */
	D3D11_BOX destRegion;
	/* 행 하나의 크기 */
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	destRegion.front = addClipIndex;
	destRegion.back = addClipIndex + 1;

	for (UINT y = 0; y < clip->FrameCount(); y++)
	{
		/* 변경할 frame의 위치  */
		destRegion.top = y;
		destRegion.bottom = y + 1;
		/* 업데이트 */
		D3D::GetDC()->UpdateSubresource
		(
			clipTexture,
			0,
			&destRegion,
			clipTransforms[addClipIndex].Transform[y],
			sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
			0
		);
	}
}

void ModelAnimator::AddSocket(int parentBoneIndex, wstring bonename)
{
	bChangeCS = true;
	UINT index = model->BoneCount();
	
	model->AddSocket(parentBoneIndex, bonename);
	
	UINT clipcount = ClipCount();

	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	//왠지 CS에서 터짐
	//destRegion.left = 4 * (index);
	//destRegion.right = 4 * (index+1);
	for (UINT x = 0; x < clipcount; x++)
	{

		ModelClip* clip = ClipByIndex(x);

		destRegion.front = x;
		destRegion.back = x + 1;

		for (UINT y = 0; y < clip->FrameCount(); y++)
		{
			memcpy(clipTransforms[x].Transform[y][index], clipTransforms[x].Transform[y][parentBoneIndex], sizeof(Matrix)); //복사
			destRegion.top = y;
			destRegion.bottom = y + 1;
			/* 업데이트 */
			D3D::GetDC()->UpdateSubresource
			(
				clipTexture,
				0,
				&destRegion,
				clipTransforms[x].Transform[y],
				sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
				0
			);
		}
	}
	ModelBone* bone = model->BoneByIndex(index);
	for (UINT x = 0; x < clipcount; x++)
	{
		Matrix global = bone->GetEditTransform()->World();
		// 월드 행렬 분해후 필요한 형태로 재조립해서 넘김
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
		/* 행 하나의 크기 */
		destRegion.left = 0;
		destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
		/* 빼올 인스턴스 행의 위치 */
		destRegion.top = x;
		destRegion.bottom = x + 1;
		destRegion.front = 0;
		destRegion.back = 1;

		/* 업데이트 */
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

void ModelAnimator::PlayAnim(UINT instance)
{
	{
		TweenDesc& desc = tweenDesc[instance];
		ModelClip* clip = ClipByIndex(desc.Curr.Clip);

		//현재 애니메이션
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

void ModelAnimator::PlayClip(UINT instance, UINT clip, float speed, float takeTime)
{
	tweenDesc[instance].TakeTime = takeTime;
	tweenDesc[instance].Next.Clip = clip;
	tweenDesc[instance].Next.Speed = speed;
}

UINT ModelAnimator::GetCurrClip(UINT instance)
{
	return tweenDesc[instance].Next.Clip>-1? tweenDesc[instance].Next.Clip: tweenDesc[instance].Curr.Clip;
}

void ModelAnimator::SetFrame(UINT instance, int frameNum)
{
	ModelClip* currClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	tweenDesc[instance].Curr.CurrFrame = frameNum % currClip->FrameCount();
	tweenDesc[instance].Curr.NextFrame = (frameNum + 1) % currClip->FrameCount();
}

UINT ModelAnimator::GetFrameCount(UINT instance)
{
	ModelClip* resultClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	if(tweenDesc[instance].Next.Clip>-1)
		resultClip = ClipByIndex(tweenDesc[instance].Next.Clip);
	return resultClip->FrameCount();
}

#pragma endregion

ModelClip * ModelAnimator::ClipByName(wstring name)
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
	이 함수가 만드는 텍스쳐는 오로지 애니메이션의 트랜스 폼만 갖게 할거임
*/
void ModelAnimator::CreateTexture()
{
	clipTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];
	for (UINT i = 0; i < ClipCount(); i++)
		CreateClipTransform(i);
	for (UINT i =  ClipCount();i< MAX_ANIMATION_CLIPS; i++)
		CreateNoClipTransform(i);


	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = MAX_ANIMATION_CLIPS;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		//void* p = malloc(pageSize * MAX_ANIMATION_CLIPS);
		void* p = VirtualAlloc(NULL, pageSize* MAX_ANIMATION_CLIPS, MEM_RESERVE, PAGE_READWRITE); //예약
		if (p == NULL)
		{
			//애니메이터 추가시 메모리 할당 오류...
			int a = 0;
		}
		for (UINT c = 0; c < MAX_ANIMATION_CLIPS; c++)
		{
			for (UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
			{
				UINT start = c * pageSize;
				void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * y * sizeof(Matrix) + start;

				VirtualAlloc(temp, MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE); //공간 할당
				memcpy(temp, clipTransforms[c].Transform[y], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //복사
			}
		}

		D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA[MAX_ANIMATION_CLIPS];
		for (UINT c = 0; c < MAX_ANIMATION_CLIPS; c++)
		{
			void* temp = (BYTE *)p + c * pageSize;

			subResource[c].pSysMem = temp;
			subResource[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
			subResource[c].SysMemSlicePitch = pageSize;
		}
		Check(D3D::GetDevice()->CreateTexture2D(&desc, subResource, &clipTexture));

		SafeDeleteArray(subResource);
		//free(p);
		VirtualFree(p, 0, MEM_RELEASE);
	}

	//CreateSRV
	{
		D3D11_TEXTURE2D_DESC desc;
		clipTexture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.ArraySize = MAX_ANIMATION_CLIPS;

		Check(D3D::GetDevice()->CreateShaderResourceView(clipTexture, &srvDesc, &clipSrv));
	}

	//for (ModelMesh* mesh : model->Meshes())
	//	mesh->TransformsSRV(clipSrv);
}

void ModelAnimator::CreateClipTransform(UINT index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS];
	
	ModelClip* clip = ClipByIndex(index);
	for (UINT f = 0; f < clip->FrameCount(); f++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++)
		{
			ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;
			///* 
			//	1. 애니메이션에서 역행렬로 들어옴 
			//	2. 키프레임 데이터는 로컬임. 그래서 부모 본을 곱해주려는것
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

void ModelAnimator::CreateNoClipTransform(UINT index)
{
	for (UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
		memcpy(clipTransforms[index].Transform[y], clipTransforms[0].Transform[0], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //복사
}

void ModelAnimator::CreateComputeDesc()
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

		for (UINT i = 0; i < outSize; i++)
			D3DXMatrixIdentity(&csOutput[i].Result);
	}
}
#pragma endregion


#pragma region 애니메이션 클립 변화 텍스쳐 생성 및 변화 구현 영역

void ModelAnimator::CreateAnimEditTexture()
{
	// 해당 버퍼가 애니메이션의 변화를 담당한다.
	// 기본 값은 identity
	for (UINT b = 0; b < model->BoneCount(); b++)
		for (UINT c = 0; c < MAX_ANIMATION_CLIPS; c++)
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

void ModelAnimator::UpdateBoneTransform(UINT part, UINT clipID, Transform* transform)
{
	if (editTexture == NULL || model == NULL)
		return;
	//클립 변화로 인한 본들의 위치 변화 재계산을 위해 CS를 다시 실행
	bChangeCS = true;

	ModelBone* bone = model->BoneByIndex(part);

	Matrix trans = transform->World();
	bone->GetEditTransform()->Local(trans);
	Matrix global = bone->GetEditTransform()->World();
	// 월드 행렬 분해후 필요한 형태로 재조립해서 넘김
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
	/* 행 하나의 크기 */
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	/* 빼올 인스턴스 행의 위치 */
	destRegion.top = clipID;
	destRegion.bottom = clipID + 1;
	destRegion.front = 0;
	destRegion.back = 1;

	/* 업데이트 */
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

void ModelAnimator::UpdateChildBones(UINT parentID, UINT childID, UINT clipID)
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