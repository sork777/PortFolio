#pragma once

/*
	�귯�� ������ ���� Ŭ����
	0820 ������ ���� Ȯ��
*/
class TerrainLod;

struct RaiseDesc
{
	Vector4 Box;

	Vector2 Res;
	Vector2 Position;

	float Radius;
	float Rate;
	float Factor;
	int RaiseType;		//0 ��� 1 �ϰ� 2 �÷�

	int SlopDir;	//1�̸� ���� 0�̸� ����
	int SlopRev;
	int SplattingLayer;
	int BrushType;
};


class TerrainBrush
{
	friend class TerrainEditor;

public:
	TerrainBrush(TerrainLod* terrain);
	~TerrainBrush();

	void Update();
	void Render();

private:
	/* �귯���� raiseDesc ������Ʈ */
	void BrushUpdater(Vector3& position);
	void BrushProperty(const bool& bSplat,bool* bRangeChanged);

private:
	Shader* shader;

	UINT terrainWidth;
	UINT terrainHeight;

	TerrainLod* curTerrainLod;

	RaiseDesc raiseDesc;
	Vector3 brushPos = Vector3(-1, -1, -1);


	float raiseRate = 50.0f;
	float rfactor = 1.0f;
	float gfactor = 1.0f;
	float sAngle = 0.0f;
private:
	struct BrushDesc
	{
		Color Color = D3DXCOLOR(0, 1, 0, 1);
		Vector3 Location;
		UINT Type = 0;
		UINT Range = 1;

		float Padding[3];
	}brushDesc;
	ConstantBuffer* brushBuffer;
	ID3DX11EffectConstantBuffer* sBrushBuffer;
};

