#include "Framework.h"
#include "OceanWater.h"

OceanWater::OceanWater(Shader * shader, UINT width, UINT height, float thick, Texture * heightMap)
	: Renderer(shader)
	, width(width), height(height), thick(thick), heightMap(heightMap)
{
	material = new Material(shader);
	material->Ambient(Color(0.25f, 0.20f, 1.0f, 1.0f));
	material->Diffuse(Color(0.600f, 0.721f, 0.937f, 0.627f));
	material->Specular(Color(1, 1, 1, 500));
	material->LoadNormalMapW(L"Environment/WaveNormal.png");

	sMap = shader->AsSRV("HeightMap");


	VertexTexture* vertices = NULL;
	UINT* indices = NULL;

	//Create VertexData
	{
		vertexCount = (width + 1) * (height + 1);
		vertices = new VertexTexture[vertexCount];

		for (UINT z = 0; z <= height; z++)
		{
			for (UINT x = 0; x <= width; x++)
			{
				UINT index = (width + 1) * z + x;

				vertices[index].Position.x = (float)x * thick;
				vertices[index].Position.y = (float)0.0f;
				vertices[index].Position.z = (float)z * thick;

				vertices[index].Uv.x = (float)x / (float)width;
				vertices[index].Uv.y = (float)z / (float)height;
			}
		}//for(z)
	}

	//CreateIndexData
	{
		indexCount = width * height * 6;
		indices = new UINT[indexCount];

		UINT index = 0;
		for (UINT z = 0; z < height; z++)
		{
			for (UINT x = 0; x < width; x++)
			{
				indices[index + 0] = (width + 1) * z + x; //0
				indices[index + 1] = (width + 1) * (z + 1) + x; //1
				indices[index + 2] = (width + 1) * z + x + 1; //2

				indices[index + 3] = (width + 1) * z + x + 1; //2
				indices[index + 4] = (width + 1) * (z + 1) + x; //1
				indices[index + 5] = (width + 1) * (z + 1) + x + 1; //1

				index += 6;
			}
		}//for(z)
	}

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);


	desc.OceanSize = Vector2(width * thick, height * thick);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Ocean");

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
}

OceanWater::~OceanWater()
{
	SafeDelete(buffer);
	SafeDelete(material);
}

void OceanWater::Update()
{
	Renderer::Update();

}

void OceanWater::Render()
{
	Renderer::Render();

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	sMap->SetResource(heightMap->SRV());

	material->Render();
	shader->DrawIndexed(0, Pass(), indexCount);
}