#include "Framework.h"
#include "MeshCube.h"

MeshCube::MeshCube()
{

}

MeshCube::~MeshCube()
{

}

void MeshCube::Create()
{
	vector<MeshVertex> v;

	float w, h, d;
	w = h = d = 0.5f;

	//Front
	v.push_back(MeshVertex(-w, -h, -d, 0, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(-w, +h, -d, 0, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, -d, 1, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(MeshVertex(+w, -h, -d, 1, 1, 0, 0, -1, 1, 0, 0));

	//Back
	v.push_back(MeshVertex(-w, -h, +d, 1, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, +d, 0, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(+w, +h, +d, 0, 0, 0, 0, 1, -1, 0, 0));
	v.push_back(MeshVertex(-w, +h, +d, 1, 0, 0, 0, 1, -1, 0, 0));

	//Top
	v.push_back(MeshVertex(-w, +h, -d, 0, 1, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(MeshVertex(+w, +h, -d, 1, 1, 0, 1, 0, 1, 0, 0));

	//Bottom
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(+w, -h, +d, 0, 0, 0, -1, 0, -1, 0, 0));
	v.push_back(MeshVertex(-w, -h, +d, 1, 0, 0, -1, 0, -1, 0, 0));

	//Left
	v.push_back(MeshVertex(-w, -h, +d, 0, 1, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, +d, 0, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, -d, 1, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(-w, -h, -d, 1, 1, -1, 0, 0, 0, 0, -1));

	//Right
	v.push_back(MeshVertex(+w, -h, -d, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, +h, -d, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, +h, +d, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(MeshVertex(+w, -h, +d, 1, 1, 1, 0, 0, 0, 0, 1));

	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<MeshVertex *>(vertices, vertexCount)
	);


	indexCount = 36;
	this->indices = new UINT[indexCount]
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	tessICount = 88;
	this->TessIndices = new UINT[tessICount]
	{
		0,  1,  3, 2,			//f
		4,  5,  7, 6,			//ba
		8,  9,  11, 10,			//t
		12, 13, 15, 14,			//bo
		16, 17, 19, 18,			//l
		20, 21, 23, 22,			//r
		//
		1,8,2,11,
		3,2,20,21,
		19,18,0,1,
		12,0,13,3,
		//
		6,10,7,9,
		23,22,5,6,
		4,7,16,17,
		14,5,15,4,
		//		
		2,11,21,10,
		21,10,22,6,
		3,20,13,23,
		13,23,14,5,
		7,9,17,8,
		17,8,18,1,
		4,16,15,19,
		15,19,12,0,
		
	};
}