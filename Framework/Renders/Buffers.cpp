#include "Framework.h"
#include "Buffers.h"

////////////////////////////////////////////////////////////////////////////////

ConstantBuffer::ConstantBuffer(void* data, UINT dataSize, bool bImute)
	:data(data), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = dataSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	
	//const�� ���� �뵵�� ����.
	desc.Usage = bImute? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = bImute? 0 : D3D11_CPU_ACCESS_WRITE;


	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = data;
	//const�� ���� �ʱ�ȭ ������ ����.
	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, bImute? &subResource:NULL, &buffer);
	Check(hr);

}


ConstantBuffer::~ConstantBuffer()
{
	SafeRelease(buffer);
}

void ConstantBuffer::Apply()
{
	//gpu�� �ֱ����� ����.
	D3D11_MAPPED_SUBRESOURCE subResource;

	D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, data, dataSize);
	}
	D3D::GetDC()->Unmap(buffer, 0);
}

////////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer(void* data,
	UINT count,
	UINT stride,
	UINT slot,
	bool bCpuWrite,
	bool bGpuWrite)
	:data(data), count(count), stride(stride), slot(slot),
	bCpuWrite(bCpuWrite), bGpuWrite(bGpuWrite)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = stride * count;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (bCpuWrite == false && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else if (bCpuWrite == true && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		//cpu->gpu�� ������ �ְڴ�?
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (bCpuWrite == false && bGpuWrite == true)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
	}
	else if (bCpuWrite == true && bGpuWrite == true)
	{
		//staging �Ⱦ��Ŵ�
		assert(false);
	}

	//�ʱ�ȭ ������ �־��ִ� ��ü
	//������ ����� ������ ������ ���� �޸� �����
	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = data;

	Check(D3D::GetDevice()->CreateBuffer(&desc, data != NULL ? &subResource : NULL, &buffer));
}


VertexBuffer::~VertexBuffer()
{
	SafeRelease(buffer);
}

void VertexBuffer::Render()
{
	//vBuffer ����
	UINT offset = 0;

	//stride�� �ּ� �ѱ�� ������ ���԰� ��������
	D3D::GetDC()->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);

}

////////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer(void* data, UINT count)
	:data(data), count(count)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = sizeof(UINT) * count;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	//�ε��� �����ʹ� ������ �����Ŷ�
	desc.Usage = D3D11_USAGE_IMMUTABLE;


	//�ʱ�ȭ ������ �־��ִ� ��ü
	//������ ����� ������ ������ ���� �޸� �����
	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = data;

	Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &buffer));

}


IndexBuffer::~IndexBuffer()
{
	SafeRelease(buffer);
}

void IndexBuffer::Render()
{
	//Buffer ����
	UINT offset = 0;

	D3D::GetDC()->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);

}

////////////////////////////////////////////////////////////////////////////////
#pragma region CS

CsResource::CsResource()
	:input(NULL),srv(NULL)
	,output(NULL), uav(NULL)
	,result(NULL)
{
}

CsResource::~CsResource()
{
}

void CsResource::Copy(void * data, UINT size)
{
	D3D::GetDC()->CopyResource(result, output);

	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(result, 0, D3D11_MAP_READ, 0, &subResource);
	{
		memcpy(data, subResource.pData, size);
	}
	D3D::GetDC()->Unmap(result, 0);
}

void CsResource::CreateBuffer()
{
	/* �����Լ��� ������ ���� �ȵǼ� */
	CreateInput();
	CreateOutput();
	CreateSRV();

	CreateUAV();

	CreateResult();
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////
#pragma region Raw

RawBuffer::RawBuffer(void * inputData, UINT byteWidth)
	:CsResource()
	,inputData(inputData),byteWidth(byteWidth)
{
	CreateBuffer();
}

RawBuffer::~RawBuffer()
{
	SafeRelease(input);
	SafeRelease(srv);

	SafeRelease(output);
	SafeRelease(uav);

	SafeRelease(result);
}

void RawBuffer::CreateInput()
{
	/* 2���� �ѱ���� �ؽ��ķ� �ѱ�� ��? */
	ID3D11Buffer* buffer = NULL;
	
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;/*RAW_VIEWS�� byteadress�� 1:1?*/
	
	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = inputData;
	D3D::GetDevice()->CreateBuffer(&desc, inputData != NULL ? &subResource : NULL,&buffer);

	input = (ID3D11Resource*)buffer;
}

void RawBuffer::CreateSRV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer*)input;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	/* ���Ƿ� ���� ����Ʈ ����? ���� �ٽú��� */
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	/* RawBuffer ���Ŷ� Ȯ���� ex��? */
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	/* offset */
	srvDesc.BufferEx.FirstElement = 0;
	/* �Ϲ� buffer�� �긦 ���� */
	srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
	/* format�� ���� ���� ���� �ٸ� */
	srvDesc.Buffer.NumElements = desc.ByteWidth / 4;
	Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &srv));
}

void RawBuffer::CreateOutput()
{
	ID3D11Buffer* buffer = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer);

	output = (ID3D11Resource*)buffer;
}

void RawBuffer::CreateUAV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer*)output;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	uavDesc.Buffer.NumElements = desc.ByteWidth / 4;

	Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav));
}

void RawBuffer::CreateResult()
{
	ID3D11Buffer* buffer=NULL;

	D3D11_BUFFER_DESC desc;
	/* �ؽ��ĸ� �ؽ��ķ�, resource���� getdesc ����. */
	((ID3D11Buffer*)input)->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer);

	result = (ID3D11Resource*)buffer;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////
#pragma region Structured

StructuredBuffer::StructuredBuffer(void * inputData, UINT stride, UINT count, bool bCopy, UINT outputStride, UINT outputCount, bool bSwap)
	: CsResource()
	, inputData(inputData), stride(stride), count(count)
	, outputStride(outputStride), outputCount(outputCount)
	, bCopy(bCopy)
	, srv2(NULL), uav2(NULL)
{
	if (outputStride == 0)
		this->outputStride = stride;

	if (outputCount == 0)
		this->outputCount = count;

	if (bCopy == false)
		bSwap = false;
	else
		this->bSwap = bSwap;

	CreateBuffer();
}

StructuredBuffer::~StructuredBuffer()
{
}

void StructuredBuffer::UpdateInput()
{
	/*D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(input, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, inputData, stride);
	}
	D3D::GetDC()->Unmap(input, 0);*/
	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right =	stride*count;
	destRegion.top = 0;
	destRegion.bottom = 1;
	destRegion.front = 0;
	destRegion.back = 1;

	/* ������Ʈ */
	D3D::GetDC()->UpdateSubresource
	(
		input,
		0,
		&destRegion,
		inputData,
		stride*count,
		0
	);
}

void StructuredBuffer::SwapData()
{
	if (bSwap == false)
		return;
	swap(srv, srv2);
	swap(uav, uav2);
}

void StructuredBuffer::CreateInput()
{
	ID3D11Buffer* buffer = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = InputByteWidth();
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = inputData;

	Check(D3D::GetDevice()->CreateBuffer(&desc, inputData != NULL ? &subResource : NULL, &buffer));

	if (bCopy)
		input = (ID3D11Resource *)buffer;
	else
		output = (ID3D11Resource *)buffer;
}

void StructuredBuffer::CreateOutput()
{
	if (bCopy == false)
		return; 
	ID3D11Buffer* buffer = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = OutputByteWidth();
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = outputStride;

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));

	output = (ID3D11Resource *)buffer;
}

void StructuredBuffer::CreateSRV()
{
	ID3D11Buffer* buffer;
	if (bCopy)
		buffer = (ID3D11Buffer *)input;
	else
		buffer = (ID3D11Buffer *)output;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;

	Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &srv));
	if (bSwap == true)
		Check(D3D::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &srv2));
}

void StructuredBuffer::CreateUAV()
{
	ID3D11Buffer* buffer = (ID3D11Buffer *)output;

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = desc.ByteWidth / desc.StructureByteStride;
	uavDesc.Buffer.Flags = 0;

	Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav));
	if (bSwap == true)
		Check(D3D::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &uav2));
}

void StructuredBuffer::CreateResult()
{
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC desc;
	((ID3D11Buffer *)output)->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));

	result = (ID3D11Buffer *)buffer;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////
#pragma region CSTex

CsTexture::CsTexture(UINT width, UINT height, DXGI_FORMAT format, bool bCpuWrite, bool bGpuWrite)
	:width(width), height(height), format(format)
	,bCpuWrite(bCpuWrite), bGpuWrite(bGpuWrite)
	
{
	if (width < 1)
		this->width = 512;
	if (height < 1)
		this->height = 512;

	CreateTex();
	CreateSRV();
	CreateUAV();
}

CsTexture::~CsTexture()
{
	SafeRelease(Tex);
	SafeRelease(srv);
	SafeRelease(uav);
}

void CsTexture::Resize(UINT width, UINT height)
{
	SafeRelease(Tex);
	SafeRelease(srv);
	SafeRelease(uav);

	// �ǹ��ִ� ���϶��� ũ�� ��ȭ�ϰ� �ƴϸ� �����ϱ�
	if (width > 0)
		this->width = width;
	if (height > 0)
		this->height = height;

	CreateTex();
	CreateSRV();
	CreateUAV();
}

void CsTexture::CreateTex()
{
	D3D11_TEXTURE2D_DESC texDesc = {
		   width,					//UINT Width;
		   height,					//UINT Height;
		   1,						//UINT MipLevels;
		   1,						//UINT ArraySize;
		   format, //DXGI_FORMAT Format;
		   1,						//DXGI_SAMPLE_DESC SampleDesc;
		   0,
		   D3D11_USAGE_DEFAULT,	//D3D11_USAGE Usage;
		   D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		   0,						//UINT CPUAccessFlags;
		   0						//UINT MiscFlags;    
	};
	if (bCpuWrite == false && bGpuWrite == false)
	{
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else if (bCpuWrite == true && bGpuWrite == false)
	{
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		//cpu->gpu�� ������ �ְڴ�?
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (bCpuWrite == false && bGpuWrite == true)
	{
		texDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	else if (bCpuWrite == true && bGpuWrite == true)
	{
		texDesc.Usage = D3D11_USAGE_STAGING;

	}

	D3D::GetDevice()->CreateTexture2D(&texDesc, NULL, &Tex);
}

void CsTexture::CreateSRV()
{

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT(format+1),
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	D3D::GetDevice()->CreateShaderResourceView(Tex, &dsrvd, &srv);
}

void CsTexture::CreateUAV()
{

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(UAVDesc));
	UAVDesc.Format = DXGI_FORMAT(format + 1);
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	D3D::GetDevice()->CreateUnorderedAccessView(Tex, &UAVDesc, &uav);
}
#pragma endregion
