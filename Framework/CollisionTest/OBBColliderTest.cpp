#include "Framework.h"
#include "OBBColliderTest.h"

OBBColliderTest::OBBColliderTest()
	:ColliderTest()
{
	Initalize();
}

OBBColliderTest::OBBColliderTest(Shader * shader, Shader * cs)
	:ColliderTest(shader,cs)
{
	Initalize();
}

OBBColliderTest::~OBBColliderTest()
{
}

void OBBColliderTest::Initalize()
{
	lineCount = 12;

	type = CollsionType::OBB;
	
	lines[0] = Vector3(-0.5f, -0.5f, -0.5f);
	lines[1] = Vector3(-0.5f, +0.5f, -0.5f);
	lines[2] = Vector3(+0.5f, -0.5f, -0.5f);
	lines[3] = Vector3(+0.5f, +0.5f, -0.5f);
	lines[4] = Vector3(-0.5f, -0.5f, +0.5f);
	lines[5] = Vector3(-0.5f, +0.5f, +0.5f);
	lines[6] = Vector3(+0.5f, -0.5f, +0.5f);
	lines[7] = Vector3(+0.5f, +0.5f, +0.5f);

	Super::Initalize();
	CreateComputeBuffer();

	rayCB = new ConstantBuffer(&rayDesc, sizeof(RayDesc));
}

void OBBColliderTest::Update()
{	
	for (int inst = 0; inst < colInfos.size(); inst++)
	{
		if (false == colInfos[inst]->bColliderOn) continue;

		colInfos[inst]->transform->Update();

		Matrix world = colInfos[inst]->transform->World();
		colInfos[inst]->transform->Render();
		if (colInfos[inst]->init != NULL)
		{
			colInfos[inst]->init->Update();
			Matrix iWorld = colInfos[inst]->init->World();
			colInfos[inst]->init->Render();
			world = iWorld * world;
		}
		csInput[inst].data = world;
	}
}

void OBBColliderTest::Render(const int& draw)
{
	if (false == bDebugMode)
		return;
	int loop = draw > 0 ? draw : colInfos.size();
	for (int inst = 0; inst < loop; inst++)
	{
		//콜라이더 사용 off면 다음것
		if (false == colInfos[inst]->bColliderOn)	continue;

		Vector3 dest[8];
		Matrix world = csInput[inst].data;

		for (UINT i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&dest[i], &lines[i], &world);

		//충돌 여부에 따라 컬러 설정
		//Color color = ()?norColor:colColor;
		Color color = norColor;

		RenderLine(dest[0], dest[1], color);
		RenderLine(dest[1], dest[3], color);
		RenderLine(dest[3], dest[2], color);
		RenderLine(dest[2], dest[0], color);


		RenderLine(dest[4], dest[5], color);
		RenderLine(dest[5], dest[7], color);
		RenderLine(dest[7], dest[6], color);
		RenderLine(dest[6], dest[4], color);


		RenderLine(dest[0], dest[4], color);
		RenderLine(dest[1], dest[5], color);
		RenderLine(dest[2], dest[6], color);
		RenderLine(dest[3], dest[7], color);
	}

	Super::Render();	
}

void OBBColliderTest::Property(const UINT & inst)
{
}


void OBBColliderTest::RayIntersect(Vector3 & position, Vector3 & direction)
{
	rayDesc.position = position;
	rayDesc.direction = direction;
	rayCB->Apply();

	csShader->AsConstantBuffer("CB_Ray")->SetConstantBuffer(rayCB->Buffer());

	obbCSBuffer->UpdateInput();
	csShader->AsSRV("Col_InputsA")->SetResource(obbCSBuffer->SRV());
	csShader->AsUAV("OBB_Outputs")->SetUnorderedAccessView(obbCSBuffer->UAV());
	csShader->Dispatch(1, 0, ceil(GetSize() / 1024.0f), 1, 1);
	obbCSBuffer->Copy(csObbOutput, sizeof(CS_ObbOutputDesc) * MAX_COLLISION);
}
const Vector3 & OBBColliderTest::GetMinRound(const UINT & inst)
{
	if (inst < colInfos.size())
		return csObbOutput[inst].MinRound;
	return Vector3();
}
const Vector3 & OBBColliderTest::GetMaxRound(const UINT & inst)
{
	if (inst < colInfos.size())
		return csObbOutput[inst].MaxRound;
	return Vector3();
}

const bool & OBBColliderTest::IsIntersectRay(OUT float& dist, const UINT & inst)
{
	if (inst < colInfos.size())
	{
		dist = csObbOutput[inst].Dist;
		return csObbOutput[inst].Intersect;
	}
	return false;
}

void OBBColliderTest::CreateComputeBuffer()
{
	Super::CreateComputeBuffer();
	UINT outSize = MAX_COLLISION;
	//csInput = new CS_InputDesc[outSize];
	csObbOutput = new CS_ObbOutputDesc[outSize];

	obbCSBuffer = new StructuredBuffer
	(
		csInput,
		sizeof(CS_InputDesc), outSize,
		true,
		sizeof(CS_ObbOutputDesc), outSize
	);
}
