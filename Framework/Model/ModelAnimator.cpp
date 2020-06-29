#include "Framework.h"
#include "ModelAnimator.h"
#include "ModelMesh.h"


ModelAnimator::ModelAnimator(const ModelAnimator & animator)
{
	// 복사생성 한다는 소리는 원본과 다른 모델로 바꿀수 있으므로 모델을 따로 갖음?
	model = new Model(*animator.model);
	shader = animator.shader;
	CloneClips(animator.clips);
	Initialize();
}

ModelAnimator::ModelAnimator(Model * model)
	: model(model)
{
	shader = model->GetShader();	

	Initialize();
}


ModelAnimator::~ModelAnimator()
{
	SafeRelease(sComputeFrameBuffer);
	SafeRelease(sUav);
	SafeDelete(computeBuffer);
	SafeDelete(csOutput);
	SafeDelete(computeShader);

	for (ModelClip* clip : clips)
		SafeDelete(clip);
	clips.clear();
	clips.shrink_to_fit();

	SafeDelete(frameBuffer);
	SafeRelease(clipTextureArray);
	SafeRelease(clipSrv);
	SafeDeleteArray(clipTransforms);
	//컴포넌트쪽에서 삭제할것
	//SafeDelete(model);

}

void ModelAnimator::Initialize()
{
	frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);

	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
	sTransformsSRV = shader->AsSRV("TransformsMap");

	computeShader = new Shader(L"PF_AttachBone.fx");

	sUav = computeShader->AsUAV("Output");
	sComputeFrameBuffer = computeShader->AsConstantBuffer("CB_AnimationFrame");

	clipTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];
}

void ModelAnimator::Update()
{
	model->Update();	
}

void ModelAnimator::Render()
{
	if (computeBuffer == NULL)
	{
		CreateComputeDesc();
	}
	
	if (clipSrv != NULL)
		sTransformsSRV->SetResource(clipSrv);

	frameBuffer->Apply();
	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	//주의, 버퍼 다 올리고 랜더하지 않으면 다른 애니메이터랑 꼬일수 있음.
	model->Render();	
}

void ModelAnimator::SetShader(Shader * shader)
{
	model->SetShader(shader);

	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelAnimator::CloneClips(const vector<ModelClip*>& oClips)
{
	//기존 객체의 클립데이터 삭제
	for (ModelClip* clip : clips)
		SafeDelete(clip);
	clips.clear();
	clips.shrink_to_fit();
	//SafeRelease(clipSrv);

	for (ModelClip* oClip : oClips)
	{
		////복사객체를 통해 데이터 복사 생성
		ModelClip* clip = new ModelClip(*oClip);
		////클립 추가후 텍스쳐 업데이트
		clips.emplace_back(clip);
		UpdateTextureArray();
	}
}

Matrix ModelAnimator::GetboneWorld(const UINT& instance, const UINT& boneIndex)
{

	if (csOutput == NULL)
	{
		Matrix temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}
	if (computeBuffer != NULL)
	{
		sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

		// 본인덱스를 통해 모델의 트랜스폼 맵에서 필요한 부분만 빼옴.
		computeShader->AsScalar("BoneIndex")->SetInt(boneIndex);
		computeShader->AsSRV("TransformsMap")->SetResource(clipSrv);
		computeShader->AsSRV("AnimEditTransformMap")->SetResource(model->GetEditSrv());
		sUav->SetUnorderedAccessView(computeBuffer->UAV());

		computeShader->Dispatch(0, 0, 1, 1, 1);
		computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_MODEL_INSTANCE);
	}

	return csOutput[instance].Result;
}

#pragma region 데이터 추가

void ModelAnimator::ReadClip(const wstring& file, const wstring& directoryPath)
{
	if (clips.size() >= MAX_ANIMATION_CLIPS)
		return;

	wstring noextfile = Path::GetFilePathWithoutExtension(file);
	wstring loadPath = directoryPath + noextfile + L".clip";

	BinaryReader* r = new BinaryReader();
	r->Open(loadPath);


	ModelClip* clip = new ModelClip();
	
	clip->name = String::ToWString(r->String());
	clip->fileName = noextfile;
	//clip->filePath = file;
	//clip->dirPath = directoryPath;

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

void ModelAnimator::SaveChangedClip(const UINT& clip, const wstring& file, const wstring& directoryPath, bool bOverwrite)
{
	//사이즈가 오버플로면
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

void ModelAnimator::AddClip(const wstring& file, const wstring& directoryPath)
{
	ReadClip(file, directoryPath);

	int addClipIndex = ClipCount() - 1;
	CreateClipTransform(addClipIndex);

	ModelClip* clip = ClipByIndex(addClipIndex);	
}

void ModelAnimator::AddSocket(const int& parentBoneIndex, const wstring& bonename)
{
	UINT index = model->BoneCount();
	
	//모델에서 소켓 생성
	model->AddSocket(parentBoneIndex, bonename);
	
	UINT clipcount = ClipCount();
	
	//각 클립에서 추가된 소켓 데이터를 갱신
	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
	//왠지 CS에서 터짐
	//destRegion.left = 4 * (index);
	//destRegion.right = 4 * (index+1);
	for(UINT x = 0; x < clipcount; x++)
	{

		ModelClip* clip = ClipByIndex(x);

		destRegion.front = x;
		destRegion.back = x + 1;

		for(UINT y = 0; y < clip->FrameCount(); y++)
		{
			memcpy(clipTransforms[x].Transform[y][index], clipTransforms[x].Transform[y][parentBoneIndex], sizeof(Matrix)); //복사
			destRegion.top = y;
			destRegion.bottom = y + 1;
			/* 업데이트 */
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
	//본 에딧 데이터 갱신.
	model->AddSocketEditData(index, clipcount);
}

#pragma endregion

#pragma region AnimationData

void ModelAnimator::SetAnimState(const AnimationState& state,const UINT & instance)
{
	tweenDesc[instance].state = state;

	if(AnimationState::Stop == state)
		SetFrame(instance, 0);
}

void ModelAnimator::PlayAnim(const UINT& instance)
{
	TweenDesc& desc = tweenDesc[instance];
	if(desc.state == AnimationState::Play)
	{

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

void ModelAnimator::PlayClip(const UINT& instance, const UINT& clip, float speed, float takeTime)
{
	tweenDesc[instance].TakeTime = takeTime;
	tweenDesc[instance].Next.Clip = clip;
	tweenDesc[instance].Next.Speed = speed;
}

UINT ModelAnimator::GetCurrClip(const UINT& instance)
{
	return tweenDesc[instance].Next.Clip>-1? tweenDesc[instance].Next.Clip: tweenDesc[instance].Curr.Clip;
}

void ModelAnimator::SetFrame(const UINT& instance, int frameNum)
{
	ModelClip* currClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	tweenDesc[instance].Curr.CurrFrame = frameNum % currClip->FrameCount();
	tweenDesc[instance].Curr.NextFrame = (frameNum + 1) % currClip->FrameCount();
}

UINT ModelAnimator::GetFrameCount(const UINT& instance)
{
	ModelClip* resultClip = ClipByIndex(tweenDesc[instance].Curr.Clip);
	if(tweenDesc[instance].Next.Clip>-1)
		resultClip = ClipByIndex(tweenDesc[instance].Next.Clip);
	return resultClip->FrameCount();
}

#pragma endregion

ModelClip * ModelAnimator::ClipByName(const wstring& name)
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
	1. 이 함수가 만드는 텍스쳐는 오로지 애니메이션의 트랜스 폼만 갖게 할거임
	2. 클립이 추가 될때마다 재생성 할거임
*/
void ModelAnimator::UpdateTextureArray()
{
	// 새로 추가된 애만 클립데이터 만들고 텍스쳐 재생성.
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
		
		void* p = VirtualAlloc(NULL, pageSize*clips.size(), MEM_RESERVE, PAGE_READWRITE); //예약

		for(UINT c = 0; c < clips.size(); c++)
		{
			for(UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
			{
				UINT start = c * pageSize;
				void* temp = (BYTE *)p + MAX_MODEL_TRANSFORMS * y * sizeof(Matrix) + start;

				VirtualAlloc(temp, MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE); //공간 할당
				memcpy(temp, clipTransforms[c].Transform[y], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //복사
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

void ModelAnimator::CreateClipTransform(const UINT& index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS];
	
	ModelClip* clip = ClipByIndex(index);
	for(UINT f = 0; f < clip->FrameCount(); f++)
	{
		for(UINT b = 0; b < model->BoneCount(); b++)
		{
			ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;
			/* 
				1. 애니메이션에서 역행렬로 들어옴 
				2. 키프레임 데이터는 로컬임. 그래서 부모 본을 곱해주려는것
			*/

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
			}
			else
			{
				bones[b] = parent;
			}		

			clipTransforms[index].Transform[f][b] = bones[b];
		}//for(b)
	}//for(f)
}

void ModelAnimator::CreateNoClipTransform(const UINT& index)
{
	for(UINT y = 0; y < MAX_MODEL_KEYFRAMES; y++)
		memcpy(clipTransforms[index].Transform[y], clipTransforms[0].Transform[0], MAX_MODEL_TRANSFORMS * sizeof(Matrix)); //복사
}

void ModelAnimator::CreateComputeDesc()
{
	UINT outSize = MAX_MODEL_INSTANCE;
	
	computeBuffer = new StructuredBuffer
	(
		NULL,
		sizeof(CS_OutputDesc), outSize,
		true
	);
	csResult = new CS_OutputDesc[outSize];

	if (csOutput == NULL)
	{
		csOutput = new CS_OutputDesc[outSize];

		for(UINT i = 0; i < outSize; i++)
			D3DXMatrixIdentity(&csOutput[i].Result);
	}
}
#pragma endregion
