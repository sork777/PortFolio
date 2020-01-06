#pragma once

////////////////////////////////////////////////////////////////////////////////

class ConstantBuffer
{
public:
	//const는 사이즈를 받아올거임??
	ConstantBuffer(void* data, UINT dataSize,bool bImute=false);
	~ConstantBuffer();

	ID3D11Buffer* Buffer() { return buffer; }

	void Apply();
private:
	ID3D11Buffer * buffer;

	//어느 데이터가 들어올지 몰라서
	void* data;
	UINT dataSize;
};

////////////////////////////////////////////////////////////////////////////////

class VertexBuffer
{
public:
	VertexBuffer(void* data,
		UINT count,
		UINT stride,
		UINT slot = 0,
		bool bCpuWrite = false,
		bool bGpuWrite = false);
	~VertexBuffer();

	UINT Count() { return count; }
	UINT Stride() { return stride; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();
private:
	ID3D11Buffer * buffer;

	//어느 데이터가 들어올지 몰라서
	void* data;
	UINT count;		//정점 갯수
	UINT stride;	//한칸의 크기
	UINT slot;

	bool bCpuWrite;
	bool bGpuWrite;
};

////////////////////////////////////////////////////////////////////////////////

class IndexBuffer
{
public:
	IndexBuffer(void* data, UINT count);
	~IndexBuffer();

	UINT Count() { return count; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();
private:
	ID3D11Buffer * buffer;

	//어느 데이터가 들어올지 몰라서
	void* data;
	UINT count;		//정점 갯수
};

////////////////////////////////////////////////////////////////////////////////

class CsResource
{
public:
	CsResource();
	virtual ~CsResource();
	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11UnorderedAccessView* UAV() { return uav; }

	void Copy(void* data, UINT size);
protected:
	virtual void CreateInput() {}
	virtual void CreateSRV() {}

	virtual void CreateOutput() {}
	virtual void CreateUAV() {}

	virtual void CreateResult() {}

	void CreateBuffer();

protected:
	/* buffer나 texture의 부모 클래스가 얘임 */
	ID3D11Resource * input;	
	ID3D11ShaderResourceView* srv;

	ID3D11Resource* output;
	/* feature lv 11 부터 이걸로 데이터를 꺼낼수 있게됨? */
	ID3D11UnorderedAccessView* uav;

	ID3D11Resource* result;
};

////////////////////////////////////////////////////////////////////////////////

class RawBuffer :public CsResource
{
public:
	RawBuffer(void* inputData, UINT byteWidth);
	~RawBuffer();

	UINT ByteWidth() { return byteWidth; }

private:
	void CreateInput() override;
	void CreateSRV() override;

	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;
private:
	void* inputData;
	UINT byteWidth;
};

////////////////////////////////////////////////////////////////////////////////

class StructuredBuffer : public CsResource
{
public:
	/* stride(데이터 하나의 크기)를 받는 이유 : 구조체 별 크기 때문 */
	StructuredBuffer(void* inputData, UINT stride, UINT count, bool bCopy=false, UINT outputStride = 0, UINT outputCount = 0);
	~StructuredBuffer();

	UINT InputByteWidth() { return stride * count; }
	UINT OutputByteWidth() { return outputStride * outputCount; }

private:
	void CreateInput() override;
	void CreateSRV() override;

	void CreateOutput() override;
	void CreateUAV() override;

	void CreateResult() override;

private:
	void* inputData;
	bool bCopy;

	UINT stride;
	UINT count;

	UINT outputStride;
	UINT outputCount;
};

////////////////////////////////////////////////////////////////////////////////
class CsTexture
{
public:
	CsTexture(UINT width=0, UINT height=0, 
		bool bCpuWrite = false,		bool bGpuWrite = true,
		DXGI_FORMAT format= DXGI_FORMAT_R8G8B8A8_TYPELESS);
	virtual ~CsTexture();

	void Resize(UINT width = 0, UINT height = 0);
	ID3D11ShaderResourceView* SRV() { return srv; }
	ID3D11UnorderedAccessView* UAV() { return uav; }
private:
	void CreateTex();
	void CreateSRV();
	void CreateUAV();

private:

	ID3D11Texture2D* Tex;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

private:
	UINT width, height;
	DXGI_FORMAT format;
	bool bCpuWrite = false;
	bool bGpuWrite = false;
};

