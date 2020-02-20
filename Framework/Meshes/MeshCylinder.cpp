#include "Framework.h"
#include "MeshCylinder.h"

MeshCylinder::MeshCylinder(float radius, float height, UINT sliceCount, UINT stackCount)
	: topRadius(radius), bottomRadius(radius), height(height), sliceCount(sliceCount), stackCount(stackCount)
{

}

MeshCylinder::~MeshCylinder()
{

}

void MeshCylinder::Create()
{
	vector<MeshVertex> vertices;

	float stackHeight = height / (float)stackCount;
	float radiusStep = (topRadius - bottomRadius) / (float)stackCount;

	UINT ringCount = stackCount + 1;
	for (UINT i = 0; i < ringCount; i++)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;
		float theta = 2.0f * Math::PI / (float)sliceCount;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			float c = cosf(k * theta);
			float s = sinf(k * theta);


			MeshVertex vertex;
			vertex.Position = D3DXVECTOR3(r * c, y, r * s);
			vertex.Uv = D3DXVECTOR2((float)k / (float)sliceCount, 1.0f - (float)i / (float)stackCount);


			D3DXVECTOR3 tangent = D3DXVECTOR3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			D3DXVECTOR3 biTangent = D3DXVECTOR3(dr * c, -height, dr * s);

			D3DXVec3Cross(&vertex.Normal, &tangent, &biTangent);
			D3DXVec3Normalize(&vertex.Normal, &vertex.Normal);

			vertex.Tangent = tangent;

			vertices.push_back(vertex);
		}
	}


	vector<UINT> indices;
	vector<UINT> indicetess;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT y = 0; y < stackCount; y++)
	{
		for (UINT x = 0; x < sliceCount; x++)
		{
			indices.push_back(y * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + (x + 1));

			indices.push_back(y * ringVertexCount + x);
			indices.push_back((y + 1) * ringVertexCount + x + 1);
			indices.push_back(y * ringVertexCount + x + 1);

			indicetess.push_back(y * ringVertexCount + x);
			indicetess.push_back((y + 1) * ringVertexCount + x);
			indicetess.push_back(y * ringVertexCount + x + 1);
			indicetess.push_back((y + 1) * ringVertexCount + x + 1);

		}
	}
	int topStart = vertices.size();
	BuildTopCap(vertices, indices, indicetess);
	int bottomStart = vertices.size();
	BuildBottomCap(vertices, indices, indicetess);

	int startindex = topStart - sliceCount;
	for (UINT k = 0; k <= sliceCount; k++)
	{
		int y = stackCount;
		int x = k;
		indicetess.push_back(y * ringVertexCount + x);
		indicetess.push_back((y + 1) * ringVertexCount + x);
		indicetess.push_back(y * ringVertexCount + x + 1);
		indicetess.push_back((y + 1) * ringVertexCount + x + 1);

		indicetess.push_back((y + 2) * ringVertexCount + x);
		indicetess.push_back(x);
		indicetess.push_back((y + 2) * ringVertexCount + x + 1);
		indicetess.push_back(x + 1);
	}


	this->vertices = new MeshVertex[vertices.size()];
	vertexCount = vertices.size();
	copy(vertices.begin(), vertices.end(), stdext::checked_array_iterator<MeshVertex *>(this->vertices, vertexCount));

	this->indices = new UINT[indices.size()];
	indexCount = indices.size();
	copy(indices.begin(), indices.end(), stdext::checked_array_iterator<UINT *>(this->indices, indexCount));

	this->TessIndices = new UINT[indicetess.size()];
	tessICount = indicetess.size();
	copy
	(
		indicetess.begin(), indicetess.end(),
		stdext::checked_array_iterator<UINT *>(this->TessIndices, tessICount)
	);
}

void MeshCylinder::BuildTopCap(vector<MeshVertex>& vertices, vector<UINT>& indices, vector<UINT>& tessindices)
{
	float y = 0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = topRadius * cosf(i * theta);
		float z = topRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, 1, 0, 1, 0, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, 1, 0, 1, 0, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i + 1);
		indices.push_back(baseIndex + i);

		tessindices.push_back(centerIndex);
		tessindices.push_back(baseIndex + i + 2);
		tessindices.push_back(baseIndex + i);
		tessindices.push_back(baseIndex + i + 1);
	}
}

void MeshCylinder::BuildBottomCap(vector<MeshVertex>& vertices, vector<UINT>& indices, vector<UINT>& tessindices)
{
	float y = -0.5f * height;
	float theta = 2.0f * Math::PI / (float)sliceCount;

	for (UINT i = 0; i <= sliceCount; i++)
	{
		float x = bottomRadius * cosf(i * theta);
		float z = bottomRadius * sinf(i * theta);

		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		vertices.push_back(MeshVertex(x, y, z, u, v, 0, -1, 0, -1, 0, 0));
	}
	vertices.push_back(MeshVertex(0, y, 0, 0.5f, 0.5f, 0, -1, 0, -1, 0, 0));


	UINT baseIndex = vertices.size() - sliceCount - 2;
	UINT centerIndex = vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(centerIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);

		tessindices.push_back(centerIndex);
		tessindices.push_back(baseIndex + i);
		tessindices.push_back(baseIndex + i + 2);
		tessindices.push_back(baseIndex + i + 1);
	}
}