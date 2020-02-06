#include "stdafx.h"
#include "CSDemo.h"

void CSDemo::Initialize()
{
	CS_Intersection();
	//CS_Structured();
}

void CSDemo::CS_Raw()
{
	Shader* shader = new Shader(L"043_ByteAddress.fx");

	struct Output
	{
		UINT GroupID[3];
		UINT GroupThreadID[3];
		UINT DispatchThreadID[3];
		UINT GroupIndex;

		//float Random;
		//float RandomOutput;
	};

	/* output갯수는 스레드 갯수랑 동일 */
	UINT size = 2* 10 * 8 * 3;
	Output* output = new Output[size];

	RawBuffer* buffer = new RawBuffer(NULL, sizeof(Output)* size);


	shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());
	/* 
	크기가 10,8,3인데 여기서 1,1,1인 이유는 
	1x1x1 사이즈에 10,8,3 크기의 스레드 그룹이 들어감 
	즉 그룹의 숫자
	*/
	shader->Dispatch(0, 0, 2, 1, 1);
	buffer->Copy(output, sizeof(Output)* size);

	FILE* file;
	fopen_s(&file, "Test.csv", "w");
	for (UINT i = 0; i < size; i++)
	{
		Output temp = output[i];

		fprintf(file,
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			i,
			temp.GroupID[0], temp.GroupID[1], temp.GroupID[2],
			temp.GroupThreadID[0], temp.GroupThreadID[1], temp.GroupThreadID[2],
			temp.DispatchThreadID[0], temp.DispatchThreadID[1], temp.DispatchThreadID[2],
			temp.GroupIndex
		);
	}
	fclose(file);
}

void CSDemo::CS_Structured()
{
	Shader* shader = new Shader(L"043_Structured.fx");

	struct Output
	{
		
		//UINT Index;
		//UINT ID[3];
		//float data;
		//float data2;
		
		UINT Index;
		UINT GroupID[3];
		UINT GroupThreadID[3];
		UINT DispatchThreadID[3];
		UINT GroupIndex;
	};

	/* output갯수는 스레드 갯수랑 동일 */
	UINT size = 2*10*8*3;
	Output* output = new Output[size];
	//for (int i = 0; i < size; i++)
	//	output[i].data = Math::Random(0.0f, 1000.0f);
	StructuredBuffer* buffer = new StructuredBuffer(output, sizeof(Output), size,true);


	shader->AsSRV("Input")->SetResource(buffer->SRV());
	shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	shader->Dispatch(0, 0,2, 1, 1);
	buffer->Copy(output, sizeof(Output)* size);

	FILE* file;
	fopen_s(&file, "TestStructuredCS2.csv", "w");
	fprintf(file,
		"i,Index,GID.x,GID.y,GID.z,GTID.x,GTID.y,GTID.z,DTID.x,DTID.y,DTID.z,GIndex\n");
	for (UINT i = 0; i < size; i++)
	{
		Output temp = output[i];
		fprintf(file,
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			i,
			temp.Index,
			temp.GroupID[0], temp.GroupID[1], temp.GroupID[2],
			temp.GroupThreadID[0], temp.GroupThreadID[1], temp.GroupThreadID[2],
			temp.DispatchThreadID[0], temp.DispatchThreadID[1], temp.DispatchThreadID[2],
			temp.GroupIndex
		);
	}
	fclose(file);
}

void CSDemo::CS_Intersection()
{
	Shader* shader = new Shader(L"043_InterSection.fx");

	struct InputDesc
	{
		UINT Index;
		Vector3 V0;
		Vector3 V1;
		Vector3 V2;
	};

	UINT width = 4;
	UINT height = 4;

	UINT index = 0;

	Vector3* vertices = new Vector3[width * height];
	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			index = z * width + x;

			vertices[index] = Vector3((float)x, 0.0f, (float)z);
		}
	}
	index = 0;
	
	UINT indexCount = (width - 1) * (height - 1) * 6;
	UINT* indices = new UINT[indexCount];
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

	InputDesc* input = new InputDesc[indexCount / 3];
	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		input[i].V0 = vertices[index0];
		input[i].V1 = vertices[index1];
		input[i].V2 = vertices[index2];

		input[i].Index = i;
	}

	struct OutputDesc
	{
		UINT Picked;
		float U;
		float V;
		float Distance;
	};


	UINT size = 1 * (indexCount / 3);
	OutputDesc* output = new OutputDesc[size];

	StructuredBuffer* buffer = new StructuredBuffer
	(
		input,
		sizeof(InputDesc), size,
		true,
		sizeof(OutputDesc), size
	);
	struct Ray
	{
		Vector3 Position;
		float Padding;

		Vector3 Direction;
		float Padding2;
	} ray;

	ray.Position = Vector3(0.6f, 0.0f, 1.6f);
	ray.Direction = Vector3(0, -1, 0);

	shader->AsVector("Position")->SetFloatVector(ray.Position);
	shader->AsVector("Direction")->SetFloatVector(ray.Direction);
	shader->AsSRV("Input")->SetResource(buffer->SRV());
	shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	shader->Dispatch(0, 0, 1, 1, 1);

	buffer->Copy(output, sizeof(OutputDesc) * size);

	FILE* file;
	fopen_s(&file, "Test.csv", "w");
	for (UINT i = 0; i < size; i++)
	{
		OutputDesc temp = output[i];

		fprintf(file,
			"%d,%.0f,%.0f,%.0f\n",
			temp.Picked,
			temp.U, temp.V, temp.Distance
		);
	}
	fclose(file);



	////////////////////////
	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			index = z * width + x;

			vertices[index] = Vector3((float)x-1, 0.0f, (float)z-1);
		}
	}
	for (UINT i = 0; i < indexCount / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		input[i].V0 = vertices[index0];
		input[i].V1 = vertices[index1];
		input[i].V2 = vertices[index2];

		input[i].Index = i;
	}
	buffer->UpdateInput();

	shader->AsVector("Position")->SetFloatVector(ray.Position);
	shader->AsVector("Direction")->SetFloatVector(ray.Direction);
	shader->AsSRV("Input")->SetResource(buffer->SRV());
	shader->AsUAV("Output")->SetUnorderedAccessView(buffer->UAV());

	shader->Dispatch(0, 0, 1, 1, 1);

	buffer->Copy(output, sizeof(OutputDesc) * size);

	fopen_s(&file, "Test2.csv", "w");
	for (UINT i = 0; i < size; i++)
	{
		OutputDesc temp = output[i];

		fprintf(file,
			"%d,%.0f,%.0f,%.0f\n",
			temp.Picked,
			temp.U, temp.V, temp.Distance
		);
	}
	fclose(file);
}
