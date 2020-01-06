#include "Framework.h"
#include "PondWater.h"


PondWater::PondWater(Shader* shader, float height, float radius)
	:Renderer(shader), height(height), radius(radius),
	waveSpeed(0.003f),mapScale(1024.0f, 1024.0f)
{
	vertexCount = 4;
	indexCount = 6;

	VertexTexture vertices[4];
	vertices[0].Position = Vector3(-radius, 0.0f, -radius);
	vertices[1].Position = Vector3(-radius, 0.0f, +radius);
	vertices[2].Position = Vector3(+radius, 0.0f, -radius);
	vertices[3].Position = Vector3(+radius, 0.0f, +radius);

	vertices[0].Uv = Vector2(0, 1);
	vertices[1].Uv = Vector2(0, 0);
	vertices[2].Uv = Vector2(1, 1);
	vertices[3].Uv = Vector2(1, 0);

	UINT indices[6] = { 0,1,2,2,1,3 };

	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(VertexTexture));
	indexBuffer = new IndexBuffer(indices, indexCount);

	waveMap = new Texture(L"Environment/Wave.dds");
	shader->AsSRV("WaveMap")->SetResource(waveMap->SRV());

	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	shader->AsConstantBuffer("CB_PondBuffer")->SetConstantBuffer(buffer->Buffer());

	{
		sClipPlane = shader->AsVector("WaterClipPlane");

		reflectionCamera = new Fixity();

		reflection = new RenderTarget((UINT)mapScale.x, (UINT)mapScale.y);
		refraction = new RenderTarget((UINT)mapScale.x, (UINT)mapScale.y);
		depthStencil = new DepthStencil((UINT)mapScale.x, (UINT)mapScale.y);
		viewport = new Viewport(mapScale.x, mapScale.y);

		shader->AsSRV("ReflectionMap")->SetResource(reflection->SRV());
		shader->AsSRV("RefractionMap")->SetResource(refraction->SRV());
	}
}


PondWater::~PondWater()
{
}

void PondWater::Update()
{
	Super::Update();

	desc.WaveTranslation += waveSpeed;
	if (desc.WaveTranslation > 1.0f)
		desc.WaveTranslation = 0.0f;

	buffer->Apply();

	GetTransform()->Position(0, height, 0);

	/* 메인 카메라의 정보, 반사광 계산하려고 */
	Vector3 position, rotation;
	Context::Get()->GetCamera()->Rotation(&rotation);
	Context::Get()->GetCamera()->Position(&position);
	
	rotation.x *= -1.0f;	
	reflectionCamera->Rotation(rotation);
	
	position.y = (height*2.0f) -position.y;
	reflectionCamera->Position(position);
	
	reflectionCamera->GetMatrix(&desc.ReflectionMatrix);

}

void PondWater::Render()
{
	Super::Render();

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shader->DrawIndexed(0, Pass(), indexCount);
}

void PondWater::SetReflection()
{
	Plane clipPlane = Plane(0, 1, 0, -height);
	sClipPlane->SetFloatVector(clipPlane);

	Context::Get()->SetSubCamera(reflectionCamera);

	reflection->Set(depthStencil->DSV());
	viewport->RSSetViewport();
}

void PondWater::SetRefraction()
{
	Plane clipPlane = Plane(0, -1, 0, height+0.1f);
	sClipPlane->SetFloatVector(clipPlane);

	Context::Get()->SetMainCamera();

	refraction->Set(depthStencil->DSV());
	viewport->RSSetViewport();
}
