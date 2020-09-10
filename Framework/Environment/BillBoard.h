#pragma once
#define MAX_BILL_INSTANCE 65536

/*
	������ Geometry�� �ϴ� ���°� ���� 1�� ����
	�ν��Ͻ��� �ʿ��ұ�? ���ٰ� ����.
	�޽õ� ó�� �������� ������ �ƴ϶� 
	���������ο� ���� �����ͷ��� �ν��Ͻ��̳� ����������...
*/

class BillBoard :public Renderer
{
public:
	BillBoard();
	~BillBoard();

	void Clear();

	void Update();
	void Render();

public:
	void FixedY(const bool& val);
	void AddInstance(Vector3& position);
	void DeleteInstance(const UINT& index);

	//�ͷ��ΰ��������� �������� ���̸� �ڵ����� �ϱ�����
	//�ε��� ���ΰ� �ƴϸ� false, ������ true
	const bool& GetBillBoardPos(const UINT& index,OUT Vector3* position);
	const bool& SetBillBoardPos(const UINT& index,const Vector3& position);
	void SetBillBoardScale(const Vector2& scale);
	void GetBillBoardScale(OUT Vector2* scale) { *scale = billScale; }
	void SetWindParm(const Vector2& windParm) {	this->windParm = windParm;	}
	
	const bool& IsMaxBill() { return (MAX_BILL_INSTANCE == drawCount); }
	UINT GetSize() { return vertices.size(); }
	void ResizeBuffer();

	void TextureSelecter(const wstring& filePath=L"");
	const bool& BillBoardButton();
	 
private:
	struct VertexScale
	{
		Vector3 Position;
	};
private:
	bool fixedY;
	bool bChangeCount;
	UINT drawCount;
	Vector2 windParm;
	Vector2 billScale;
	float time;

	vector<VertexScale> vertices;

	Texture* billTex = NULL;
	ID3D11ShaderResourceView* billSrv=NULL;
	ID3DX11EffectShaderResourceVariable* sSRV=NULL;
};

