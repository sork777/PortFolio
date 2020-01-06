#pragma once

class Rain : public Renderer
{
public:
	Rain(Vector3 extent, UINT count);
	~Rain();

	void Update();
	void Render();

private:
	struct Desc
	{
		D3DXCOLOR Color = D3DXCOLOR(1,1,1,1);
		Vector3 Velocity = Vector3(-10, -100, 0);
		float DrawDistance = 1000.0f;

		//�� ���� ��ġ
		Vector3 Origin = Vector3(0, 0, 0);
		float Padding;

		//���� ����? ť�� ����
		Vector3 Extent = Vector3(0, 0, 0);
		float Padding2;
	}desc;
	
private:
	//�����ڷ���
	struct VertexRain
	{
		Vector3 Position;
		Vector2 Uv;
		Vector2 Scale;
	};
private:
	ConstantBuffer * buffer;
	ID3DX11EffectConstantBuffer* sBuffer;

	VertexRain* vertices;
	UINT* indices;

	Texture* texture;

	UINT drawCount;
};

