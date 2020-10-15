#include "Framework.h"
#include "ModelAnimator.h"
#include "ModelMesh.h"

ModelAnimator::ModelAnimator(const ModelAnimator & animator)
{
	// ������� �Ѵٴ� �Ҹ��� ������ �ٸ� �𵨷� �ٲܼ� �����Ƿ� ���� ���� ����?
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
	//SafeDelete(computeShader);

	for (ModelClip* clip : clips)
		SafeDelete(clip);
	clips.clear();
	clips.shrink_to_fit();

	SafeDelete(frameBuffer);
	SafeRelease(clipTextureArray);
	SafeRelease(clipSrv);
	SafeDeleteArray(clipTransforms);
	//������Ʈ�ʿ��� �����Ұ�
	//SafeDelete(model);
}

void ModelAnimator::Initialize()
{
	frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);

	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
	sTransformsSRV = shader->AsSRV("TransformsMap");

	computeShader = SETSHADER(L"PF_AttachBone.fx");

	sUav = computeShader->AsUAV("Output");
	sComputeFrameBuffer = computeShader->AsConstantBuffer("CB_AnimationFrame");

	clipTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];

	testCSTex = new CsTexture(model->BoneCount()*4,1, DXGI_FORMAT_R32G32B32A32_TYPELESS);
}

void ModelAnimator::Update()
{
	if (computeBuffer == NULL)
	{
		CreateComputeDesc();
	}
	
	
	//model->Update();	
}

void ModelAnimator::Render()
{
	//����, ���� �� �ø��� �������� ������ �ٸ� �ִϸ����Ͷ� ���ϼ� ����.
	frameBuffer->Apply();
	if (model->GetPass() == 2)
	{
		if (clipSrv != NULL)
			sTransformsSRV->SetResource(clipSrv);
		sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

	}
	else
	{
		//TODO: 1006 CS ����� ������� �ذ��ؾ��� ��
		// 1. �ִϸ��̼� ���� ��Ȯ���� ���� clip�����Ͱ� ���Ѽ� ����
		// 2. ���� �����ʿ��� ���� �ȵ� ���۵忡�� �ȵǴ°ǰ�.
		// 3. 1012 actor �Ŵ����ʿ� �̻������� Ȯ��
		sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

		if (model->IsAdded())
		{
			testCSTex->Resize(model->BoneCount() * 4, model->GetInstSize());
		}

		Vector2 size = testCSTex->GetSize();
		UINT dispatchX = ceil((size.x / 4 * size.y) / 1024);
		computeShader->AsScalar("TexWidth")->SetInt(size.x);

		computeShader->AsSRV("TransformsMap")->SetResource(clipSrv);
		computeShader->AsSRV("AnimEditTransformMap")->SetResource(model->GetEditSrv());
		computeShader->AsUAV("OutputMap")->SetUnorderedAccessView(testCSTex->UAV());

		computeShader->Dispatch(0, 1, dispatchX, 1, 1);
		shader->AsSRV("AnimTransformTestMap")->SetResource(testCSTex->SRV());
	}
	
	//model->Render();	
}

void ModelAnimator::SetShader(Shader * shader)
{
	model->SetShader(shader);

	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelAnimator::CloneClips(const vector<ModelClip*>& oClips)
{
	//���� ��ü�� Ŭ�������� ����

	int size = clips.size();
	if (size > 0)
	{
		for (ModelClip* clip : clips)
			SafeDelete(clip);
	}
	clips.clear();
	clips.shrink_to_fit();

	for (ModelClip* oClip : oClips)
	{
		////���簴ü�� ���� ������ ���� ����
		ModelClip* clip = new ModelClip(*oClip);
		////Ŭ�� �߰��� �ؽ��� ������Ʈ
		clips.emplace_back(clip);
		maxAnimFrame = maxAnimFrame > clip->frameCount ? maxAnimFrame : clip->frameCount;
		UpdateTextureArray();
	}
}

void ModelAnimator::ReadyforGetBoneworld(const UINT & boneIndex)
{
	if (computeBuffer != NULL)
	{
		sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

		// ���ε����� ���� ���� Ʈ������ �ʿ��� �ʿ��� �κи� ����.
		computeShader->AsScalar("BoneIndex")->SetInt(boneIndex);
		computeShader->AsSRV("TransformsMap")->SetResource(clipSrv);
		computeShader->AsSRV("AnimEditTransformMap")->SetResource(model->GetEditSrv());
		sUav->SetUnorderedAccessView(computeBuffer->UAV());

		computeShader->Dispatch(0, 0, 1, 1, 1);
		computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_MODEL_INSTANCE);
	}
}
Matrix ModelAnimator::GetboneWorld(const UINT& instance)
{
	if (csOutput == NULL)
	{
		Matrix temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}

	return csOutput[instance].Result;
}

#pragma region ������ �߰�

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

	maxAnimFrame = maxAnimFrame > clip->frameCount ? maxAnimFrame : clip->frameCount;

	UpdateTextureArray();
}

void ModelAnimator::SaveChangedClip(const UINT& clip, const wstring& file, const wstring& directoryPath, bool bOverwrite)
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

	ModelClip* saveclip = clips[clip];

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->String(String::ToString(saveclip->name));
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
			editedData.Time = data.Time;
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

	//�𵨿��� ���� ����
	model->AddSocket(parentBoneIndex, bonename);

	UINT clipcount = ClipCount();

	//�� Ŭ������ �߰��� ���� �����͸� ����
	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = 4 * model->BoneCount();
	//���� CS���� ����
	//destRegion.left = 4 * (index);
	//destRegion.right = 4 * (index+1);
	for (UINT x = 0; x < clipcount; x++)
	{

		ModelClip* clip = ClipByIndex(x);

		destRegion.front = x;
		destRegion.back = x + 1;

		for (UINT y = 0; y < clip->FrameCount(); y++)
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
				sizeof(Matrix)*model->BoneCount(),
				0
			);
		}
	}
	//�� ���� ������ ����.
	model->AddSocketEditData(index, clipcount);
}

#pragma endregion

#pragma region AnimationData

void ModelAnimator::SetAnimState(const AnimationState& state, const UINT & instance)
{
	tweenDesc[instance].state = state;

	if (AnimationState::Stop == state)
		SetFrame(instance, 0);
}

void ModelAnimator::PlayAnim(const UINT& instance)
{
	TweenDesc& desc = tweenDesc[instance];
	if (desc.state == AnimationState::Play)
	{

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

void ModelAnimator::PlayClip(const UINT& instance, const UINT& clip, float speed, float takeTime)
{
	tweenDesc[instance].TakeTime = takeTime;
	tweenDesc[instance].Next.Clip = clip;
	tweenDesc[instance].Next.Speed = speed;
}

UINT ModelAnimator::GetCurrClip(const UINT& instance)
{
	return tweenDesc[instance].Next.Clip > -1 ? tweenDesc[instance].Next.Clip : tweenDesc[instance].Curr.Clip;
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
	if (tweenDesc[instance].Next.Clip > -1)
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
	1. �� �Լ��� ����� �ؽ��Ĵ� ������ �ִϸ��̼��� Ʈ���� ���� ���� �Ұ���
	2. Ŭ���� �߰� �ɶ����� ����� �Ұ���
*/
void ModelAnimator::UpdateTextureArray()
{
	// ���� �߰��� �ָ� Ŭ�������� ����� �ؽ��� �����.
	UINT c = clips.size() - 1;
	CreateClipTransform(c);

	SafeRelease(clipTextureArray);
	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = model->BoneCount() * 4;
		desc.Height = maxAnimFrame;
		desc.ArraySize = clips.size();
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		UINT pageSize = model->BoneCount() * 4 * 16 * maxAnimFrame;

		void* p = VirtualAlloc(NULL, pageSize*clips.size(), MEM_RESERVE, PAGE_READWRITE); //����

		for (UINT c = 0; c < clips.size(); c++)
		{
			for (UINT y = 0; y < maxAnimFrame; y++)
			{
				UINT start = c * pageSize;
				void* temp = (BYTE *)p + model->BoneCount() * y * sizeof(Matrix) + start;

				VirtualAlloc(temp, model->BoneCount() * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE); //���� �Ҵ�
				memcpy(temp, clipTransforms[c].Transform[y], model->BoneCount() * sizeof(Matrix)); //����
			}
		}

		D3D11_SUBRESOURCE_DATA* subResource = new D3D11_SUBRESOURCE_DATA[clips.size()];
		for (UINT c = 0; c < clips.size(); c++)
		{
			void* temp = (BYTE *)p + c * pageSize;

			subResource[c].pSysMem = temp;
			subResource[c].SysMemPitch = model->BoneCount() * sizeof(Matrix);
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
	Matrix* bones = new Matrix[model->BoneCount()];

	ModelClip* clip = ClipByIndex(index);
	for (UINT f = 0; f < clip->FrameCount(); f++)
	{
		for (UINT b = 0; b < model->BoneCount(); b++)
		{
			ModelBone* bone = model->BoneByIndex(b);

			Matrix parent;
			/*
				1. �ִϸ��̼ǿ��� ����ķ� ����
				2. Ű������ �����ʹ� ������. �׷��� �θ� ���� �����ַ��°�
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
	for (UINT y = 0; y < maxAnimFrame; y++)
		memcpy(clipTransforms[index].Transform[y], clipTransforms[0].Transform[0], model->BoneCount() * sizeof(Matrix)); //����
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

		for (UINT i = 0; i < outSize; i++)
			D3DXMatrixIdentity(&csOutput[i].Result);
	}
}

#pragma endregion
