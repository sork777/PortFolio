#include "Framework.h"
#include "MeshQuad.h"

MeshQuad::MeshQuad()
{
	type = MeshType::Quad;

}

MeshQuad::MeshQuad(const MeshQuad & mesh)
	:Mesh(mesh)
{
	type = MeshType::Quad;
}

MeshQuad::~MeshQuad()
{

}

void MeshQuad::Create()
{
	float w = 0.5f;
	float h = 0.5f;

	vector<MeshVertex> v;
	v.push_back(MeshVertex(-w, -h, 0, 0, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(-w, +h, 0, 0, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, -h, 0, 1, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, 0, 1, 0, 0, 0, -1, 1, 0, 0));

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount)
	);

	indexCount = 6;
	indices = new UINT[indexCount]{ 0, 1, 2, 2, 1, 3 };


	/*tessICount = 4;
	this->TessIndices = new UINT[tessICount]{ 0, 1, 3, 2 };*/
}
