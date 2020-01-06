#pragma once

class Snow : public Renderer
{
public:
	Snow(Vector3 extent, UINT count);
	~Snow();

	void Update();
	void Render();

private:
	struct Desc
	{
		D3DXCOLOR Color = D3DXCOLOR(1,1,1,1);
		Vector3 Velocity = Vector3(-10, -10, 0);
		float DrawDistance = 1000.0f;

		//비 내릴 위치
		Vector3 Origin = Vector3(0, 0, 0);
		float Tabulence = 5;

		//비의 범위? 큐브 공간
		Vector3 Extent = Vector3(0, 0, 0);
		float Padding;
	}desc;
	
private:
	//정점자료형
	struct VertexSnow
	{
		Vector3 Position;
		Vector2 Uv;
		float Scale;
		//랜덤 흔들림
		Vector2 Random;
	};
private:
	ConstantBuffer * buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	VertexSnow* vertices;
	UINT* indices;

	Texture* texture;

	UINT drawCount;
};

