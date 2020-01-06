#include "Framework.h"
#include "Snow.h"


Snow::Snow(Vector3  extent, UINT count)
	:Renderer(L"023_Snow.fx")
	,drawCount(count)
{
	desc.Extent = extent;
	texture = new Texture(L"Environment/Snow.png");
	//공유할것도 아니고 
	shader->AsSRV("DiffuseMap")->SetResource(texture->SRV());
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_Snow");

	vertices = new VertexSnow[drawCount * 4];
	indices = new UINT[drawCount * 6];

	for (UINT i = 0; i < drawCount * 4; i+=4)
	{
		float scale;
		scale = Math::Random(0.2f, 0.6f);

		Vector3 position;
		position.x = Math::Random(-desc.Extent.x, desc.Extent.x);
		position.y = Math::Random(-desc.Extent.y, desc.Extent.y);
		position.z = Math::Random(-desc.Extent.z, desc.Extent.z);

		Vector2 random;
		random.x = Math::Random(0.0f, 1.0f);
		random.y = Math::Random(0.0f, 1.0f);

		vertices[i + 0].Position = position;
		vertices[i + 1].Position = position;
		vertices[i + 2].Position = position;
		vertices[i + 3].Position = position;

		vertices[i + 0].Uv = Vector2(0,1);
		vertices[i + 1].Uv = Vector2(0,0);
		vertices[i + 2].Uv = Vector2(1,1);
		vertices[i + 3].Uv = Vector2(1,0);

		vertices[i + 0].Scale = scale;
		vertices[i + 1].Scale = scale;
		vertices[i + 2].Scale = scale;
		vertices[i + 3].Scale = scale;

		vertices[i + 0].Random = random;
		vertices[i + 1].Random = random;
		vertices[i + 2].Random = random;
		vertices[i + 3].Random = random;
	}

	for (UINT i = 0; i < drawCount; i++)
	{
		indices[i * 6 + 0] = i * 4 + 0;
		indices[i * 6 + 1] = i * 4 + 1;
		indices[i * 6 + 2] = i * 4 + 2;
		indices[i * 6 + 3] = i * 4 + 2;
		indices[i * 6 + 4] = i * 4 + 1;
		indices[i * 6 + 5] = i * 4 + 3;
	}

	vertexBuffer = new VertexBuffer(vertices, drawCount * 4, sizeof(VertexSnow));
	indexBuffer = new IndexBuffer(indices, drawCount * 6);

}

Snow::~Snow()
{
	SafeDelete(buffer);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);
	SafeDelete(texture);

}

void Snow::Update()
{
	Super::Update();
	ImGui::Separator();
	ImGui::Text("Snow");

	ImGui::SliderFloat("Origin", (float*)&desc.Origin, 0, 200);
	ImGui::InputFloat3("Velocity", (float*)&desc.Velocity, 0.1f);
	ImGui::ColorEdit3("Color", (float*)&desc.Color);
	ImGui::InputFloat("Tubulence", (float*)&desc.Tabulence, 0.1f);

}

void Snow::Render()
{
	Super::Render();

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());

	shader->DrawIndexed(0, 0, drawCount * 6);

}
