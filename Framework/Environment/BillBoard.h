#pragma once
#define MAX_BILL_INSTANCE 65536

/*
	어차피 Geometry로 하니 들어가는건 정점 1개 정보
	인스턴싱이 필요할까? 없다고 생각.
	메시들 처럼 정점들이 많은게 아니라 
	파이프라인에 들어가는 데이터량은 인스턴싱이나 도찐개찐같은데...
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

	//터레인같은곳에서 빌보드의 높이를 자동조절 하기위함
	//인덱스 내부가 아니면 false, 맞으면 true
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

