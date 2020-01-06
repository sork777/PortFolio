#include "Framework.h"
#include "ModelAnimator.h"
#include "ModelMesh.h"

ModelAnimator::ModelAnimator(Shader * shader)
	: Model(shader)
{
	frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc) * MAX_MODEL_INSTANCE);
	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");


	computeShader = new Shader(L"030_Collider.fx");
	
	sSrv = computeShader->AsSRV("Input");
	sUav = computeShader->AsUAV("Output");
	sComputeFrameBuffer = computeShader->AsConstantBuffer("CB_AnimationFrame");
}

ModelAnimator::~ModelAnimator()
{
	SafeDelete(computeShader);
	SafeDelete(computeBuffer);
	SafeRelease(sComputeFrameBuffer);
	SafeRelease(sUav);
	SafeRelease(sSrv);
	SafeDelete(csOutput);
	SafeDelete(csInput);

	SafeDelete(frameBuffer);
	SafeDeleteArray(clipTransforms);
}

void ModelAnimator::Update(UINT instance,bool bPlay)
{
	Super::Update();
	if (bPlay)
		PlayAnim(instance);
}

void ModelAnimator::Render()
{
	if (texture == NULL)
	{
		CreateComputeDesc();
	}

	Super::Render();
	frameBuffer->Apply();

	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());
	if (computeBuffer != NULL)
	{
		sComputeFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

		computeShader->AsSRV("TransformsMap")->SetResource(srv);
		sSrv->SetResource(computeBuffer->SRV());
		sUav->SetUnorderedAccessView(computeBuffer->UAV());

		computeShader->Dispatch(0, 0, GetInstSize(), 1, 1);
		computeBuffer->Copy(csOutput, sizeof(CS_OutputDesc) * MAX_MODEL_TRANSFORMS*MAX_MODEL_INSTANCE);
	}
	}

#pragma region 데이터 추가

void ModelAnimator::AddClip(wstring file)
{
	ReadClip(file);

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
			texture,
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
	UINT index = BoneCount();
	Super::AddSocket(parentBoneIndex, bonename);
	UINT clipcount = ClipCount();
	for (UINT x = 0; x < clipcount; x++)
	{
		ModelClip* clip = ClipByIndex(x);

		for (UINT y = 0; y < clip->FrameCount(); y++)
			memcpy(clipTransforms[x].Transform[y][index], clipTransforms[x].Transform[y][parentBoneIndex], sizeof(Matrix)); //복사
	}
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
			destRegion.top = y;
			destRegion.bottom = y + 1;
			/* 업데이트 */
			D3D::GetDC()->UpdateSubresource
			(
				texture,
				0,
				&destRegion,
				clipTransforms[x].Transform[y],
				sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
				0
			);
		}
	}
}

#pragma endregion

#pragma region AnimationData

void ModelAnimator::PlayAnim(UINT instance)
{
	for (UINT i = 0; i < transforms.size(); i++)
	{
		if (i == instance) continue;
		TweenDesc& desc = tweenDesc[i];
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

#pragma region TransformRegion

Matrix ModelAnimator::GetboneTransform(UINT instance, UINT boneIndex)
{

	if (csOutput == NULL)
	{
		Matrix temp;
		D3DXMatrixIdentity(&temp);

		return temp;
	}

	UINT index = instance * MAX_MODEL_TRANSFORMS + boneIndex;
	Matrix result = csOutput[index].Result;

	Matrix transform = BoneByIndex(boneIndex)->Transform();
	Matrix world = GetTransform(instance)->World();

	return transform * result * world;
}

void ModelAnimator::UpdateInstTransform(UINT instance, UINT part, Matrix trans)
{
	if (texture == NULL)
		return;

	ModelBone* bone = BoneByIndex(part);
	int clipIdx = tweenDesc[instance].Curr.Clip;
	ModelClip* clip = ClipByIndex(clipIdx);
	{
		Matrix invGlobal = bone->Transform();
		D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

		for (int f = 0; f < clip->FrameCount(); f++)
		{
			{
				Matrix parent;
				int parentIndex = bone->ParentIndex();
				if (parentIndex < 0)
				{
					D3DXMatrixIdentity(&parent);
				}
				else
				{
					parent = boneAinTransforms[clipIdx].Transform[f][parentIndex];
				}

				Matrix animation;
				ModelKeyframe* frame = clip->Keyframe(bone->Name());
				if (frame != NULL)
				{
					ModelKeyframeData data = frame->Transforms[f];

					Matrix S, R, T;
					D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
					D3DXMatrixRotationQuaternion(&R, &data.Rotation);
					D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

					animation = S * R * T;

					clipTransforms[clipIdx].Transform[f][part] = invGlobal * trans * animation * parent;
				}
				else
				{
					clipTransforms[clipIdx].Transform[f][part] = bone->Transform() *trans* parent;
				}

				for (ModelBone* child: bone->Childs())
				{
					UpdateChildTransform(part,child->Index(), clipIdx, f, trans);
				}
			}
			
			/* 변환 해줄 텍스쳐 데이터의 박스 영역 */
			D3D11_BOX destRegion;
			/* 행 하나의 크기 */
			destRegion.left = 0;
			destRegion.right = 4 * MAX_MODEL_TRANSFORMS;
			/* 빼올 인스턴스 행의 위치 */
			destRegion.top = f;
			destRegion.bottom = f + 1;
			destRegion.front = clipIdx;
			destRegion.back = clipIdx+1;

			/* 업데이트 */
			D3D::GetDC()->UpdateSubresource
			(
				texture,
				0,
				&destRegion,
				clipTransforms[clipIdx].Transform[f],
				sizeof(Matrix)*MAX_MODEL_TRANSFORMS,
				0
			);
		}
	}
}

void ModelAnimator::UpdateChildTransform(UINT parentID, UINT childID, UINT clipID, UINT frameID, Matrix trans)
{
	Matrix parent = boneAinTransforms[clipID].Transform[frameID][parentID];

	Matrix animation;
	ModelBone* bone = BoneByIndex(childID);
	Matrix invGlobal = bone->Transform();
	D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

	ModelClip* clip = ClipByIndex(clipID);
	ModelKeyframe* frame = clip->Keyframe(bone->Name());
	if (frame != NULL)
	{
		ModelKeyframeData data = frame->Transforms[frameID];

		Matrix S, R, T;
		D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
		D3DXMatrixRotationQuaternion(&R, &data.Rotation);
		D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

		animation = S * R * T;

		clipTransforms[clipID].Transform[frameID][childID] = invGlobal *  animation * parent*trans;
	}
	else
	{
		clipTransforms[clipID].Transform[frameID][childID] = bone->Transform()* parent*trans;
	}

	for (ModelBone* child: bone->Childs())
	{
		UpdateChildTransform(childID, child->Index(), clipID, frameID, trans);
	}
}

#pragma endregion

#pragma region CreateDataRegion
void ModelAnimator::CreateTexture()
{
	clipTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];
	boneAinTransforms = new ClipTransform[MAX_ANIMATION_CLIPS];
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
		void* p = VirtualAlloc(NULL, pageSize* MAX_ANIMATION_CLIPS, MEM_RESERVE, PAGE_READWRITE); //예약

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
		Check(D3D::GetDevice()->CreateTexture2D(&desc, subResource, &texture));

		SafeDeleteArray(subResource);
		VirtualFree(p, 0, MEM_RELEASE);
	}

	//CreateSRV
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.ArraySize = MAX_ANIMATION_CLIPS;

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	for (ModelMesh* mesh : Meshes())
		mesh->TransformsSRV(srv);
}

void ModelAnimator::CreateClipTransform(UINT index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS];
	
	ModelClip* clip = ClipByIndex(index);
	for (UINT f = 0; f < clip->FrameCount(); f++)
	{
		for (UINT b = 0; b < BoneCount(); b++)
		{
			ModelBone* bone = BoneByIndex(b);

			Matrix parent;
			Matrix invGlobal = bone->Transform();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal);

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
				clipTransforms[index].Transform[f][b] = invGlobal * bones[b];
			}
			else
			{
				bones[b] = parent;
				clipTransforms[index].Transform[f][b] = bone->Transform() * bones[b];
			}
			boneAinTransforms[index].Transform[f][b] = bones[b];
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
