#include "Framework.h"
#include "Mirror.h"


Mirror::Mirror(Shader* shader, float w, float h, float d)
	:Renderer(shader)
	, w(w), h(h), d(d)
	, mapScale(1024.0f, 1024.0f)
{
	vector<mirrorV> v;

	//Front
	v.push_back(mirrorV(-w, -h, -d, 0, 1, 0, 0, -1, 1, 0, 0));
	v.push_back(mirrorV(-w, +h, -d, 0, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(mirrorV(+w, +h, -d, 1, 0, 0, 0, -1, 1, 0, 0));
	v.push_back(mirrorV(+w, -h, -d, 1, 1, 0, 0, -1, 1, 0, 0));

	//Back
	v.push_back(mirrorV(-w, -h, +d, 1, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(mirrorV(+w, -h, +d, 0, 1, 0, 0, 1, -1, 0, 0));
	v.push_back(mirrorV(+w, +h, +d, 0, 0, 0, 0, 1, -1, 0, 0));
	v.push_back(mirrorV(-w, +h, +d, 1, 0, 0, 0, 1, -1, 0, 0));

	//Top
	v.push_back(mirrorV(-w, +h, -d, 0, 1, 0, 1, 0, 1, 0, 0));
	v.push_back(mirrorV(-w, +h, +d, 0, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(mirrorV(+w, +h, +d, 1, 0, 0, 1, 0, 1, 0, 0));
	v.push_back(mirrorV(+w, +h, -d, 1, 1, 0, 1, 0, 1, 0, 0));

	//Bottom
	v.push_back(mirrorV(-w, -h, -d, 1, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(mirrorV(+w, -h, -d, 0, 1, 0, -1, 0, -1, 0, 0));
	v.push_back(mirrorV(+w, -h, +d, 0, 0, 0, -1, 0, -1, 0, 0));
	v.push_back(mirrorV(-w, -h, +d, 1, 0, 0, -1, 0, -1, 0, 0));

	//Left
	v.push_back(mirrorV(-w, -h, +d, 0, 1, -1, 0, 0, 0, 0, -1));
	v.push_back(mirrorV(-w, +h, +d, 0, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(mirrorV(-w, +h, -d, 1, 0, -1, 0, 0, 0, 0, -1));
	v.push_back(mirrorV(-w, -h, -d, 1, 1, -1, 0, 0, 0, 0, -1));

	//Right
	v.push_back(mirrorV(+w, -h, +d*0.5f, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, +d*0.5f, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, +d, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, -h, +d, 1, 1, 1, 0, 0, 0, 0, 1));

	v.push_back(mirrorV(+w, -h, -d, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, -d, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, -d * 0.5f, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, -h, -d * 0.5f, 1, 1, 1, 0, 0, 0, 0, 1));
	
	v.push_back(mirrorV(+w, +h * 0.75f, -d * 0.5f, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, -d * 0.5f, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h, +d * 0.5f, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h * 0.75f, +d * 0.5f, 1, 1, 1, 0, 0, 0, 0, 1));

	//Mirror
	v.push_back(mirrorV(+w, -h, -d * 0.5f, 0, 1, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h * 0.75f, -d * 0.5f, 0, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, +h * 0.75f, +d * 0.5f, 1, 0, 1, 0, 0, 0, 0, 1));
	v.push_back(mirrorV(+w, -h, +d * 0.5f, 1, 1, 1, 0, 0, 0, 0, 1));

	vertices = new mirrorV[v.size()];
	vertexCount = v.size();

	copy
	(
		v.begin(), v.end(),
		stdext::checked_array_iterator<mirrorV *>(vertices, vertexCount)
	);

	indexCount = 54;
	this->indices = new UINT[indexCount]
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
		24, 25, 26, 24, 26, 27,
		28, 29, 30, 28, 30, 31,
		32, 33, 34, 32, 34, 35
	};


	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(mirrorV));
	indexBuffer = new IndexBuffer(indices, indexCount);

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	shader->AsConstantBuffer("CB_MirrorBuffer")->SetConstantBuffer(buffer->Buffer());

	{
		sClipPlane = shader->AsVector("MirrorClipPlane");

		depthStencil = new DepthStencil((UINT)mapScale.x, (UINT)mapScale.y, true);
		viewport = new Viewport(mapScale.x, mapScale.y);

		mirrorImg = new Texture(L"ice.dds");

		shader->AsSRV("MirrorMap")->SetResource(mirrorImg->SRV());
	}
}


Mirror::~Mirror()
{
}

void Mirror::Update()
{
	Super::Update();
	buffer->Apply();

	Vector3 mirror_Pos;
	GetTransform()->Position(&mirror_Pos);

	Plane clipPlane = Plane(1, 0, 0, -mirror_Pos.x-w);
	sClipPlane->SetFloatVector(clipPlane);

	Matrix R;
	D3DXMatrixReflect(&R, &clipPlane);
	shader->AsMatrix("R")->SetMatrix(R);
}

void Mirror::Render()
{
	Super::Render();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, Pass(), indexCount - 6);
}

void Mirror::MirrorRender()
{
	Super::Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, Pass(), 6, indexCount - 6);
}
